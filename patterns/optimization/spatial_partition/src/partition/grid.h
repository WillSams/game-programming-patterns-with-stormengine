#pragma once

#include <cstddef>
#include <vector>

// ── Spatial Partition pattern: ask the neighbors, not the world ───────────────
//
// The chapter's problem: a unit that wants to know what is near it, or a physics
// step that wants to test every pair, reads every object in the world. A SPATIAL
// PARTITION puts the objects into buckets, so a question about one place only
// touches the objects that are there. The arithmetic per pair does not change --
// the number of pairs does.
//
// This is a uniform grid: a fixed NxN lattice over the world, one bucket per cell,
// and every entity in exactly one bucket. It is the simplest partition that
// carries the pattern's whole idea, and the one the chapter builds.
//
// ⚠️ AND ITS SILENT BUG IS A STALE BUCKET. An entity that moves has to leave the
// bucket it was in; if it does not, it is still found THERE and not found where it
// actually is. Nothing crashes, nothing warns -- a query simply returns the object
// from the place the object used to be. It is the same shape of failure as a
// missed dirty mark: the answer is wrong and everything looks fine. So the specs
// check the placement after a CROSSING, not only after an insert, and check that
// an entity is in exactly one bucket and never in two.
//
// What it deliberately does not do:
//   * The grid is FIXED SIZE. Moving entities around does not resize it, and there
//     is no adaptive subdivision -- that would be a tree, which is a different
//     chapter. Fixed is the trade this pattern makes, and a bad cell size is the
//     cost of the pattern rather than a bug in it.
//   * `CandidatesAround` walks every entity in the cells it names; it does not do a
//     circle test. A caller that wants "within radius r, actually" filters the
//     candidates it gets. The partition's job is to make that list small.
namespace partition {

class Grid {
public:
    // `cells` per side, over a world that is the normalized unit square. `entities`
    // is the number of ids the grid will ever see; ids are 0..entities-1.
    Grid(int cells, int entities)
        : cells_(cells > 0 ? cells : 0),
          cellOf_(static_cast<std::size_t>(entities > 0 ? entities : 0), kNotInGrid) {
        buckets_.resize(CellCount());
    }

    int CellsPerSide() const { return cells_; }
    std::size_t CellCount() const {
        return static_cast<std::size_t>(cells_) * static_cast<std::size_t>(cells_);
    }

    // ⚠️ OUT OF THE WORLD IS NOT CLAMPED INTO AN EDGE CELL. A position outside [0,1)
    // belongs to no cell, and every method that takes one says so by returning false
    // or -1. Clamping would quietly file an escaped entity into the edge bucket,
    // where a query would find it as a neighbor it is nowhere near.
    int CellIndex(float x, float y) const {
        if (cells_ <= 0 || !InWorld(x) || !InWorld(y))
            return -1;
        const int cx = static_cast<int>(x * static_cast<float>(cells_));
        const int cy = static_cast<int>(y * static_cast<float>(cells_));
        return CellIndexFor(cx, cy);
    }

    int CellOf(int id) const { return Valid(id) ? cellOf_[static_cast<std::size_t>(id)] : -1; }

    // Put an entity into the bucket its position names. Returns false for an
    // out-of-world position or an unknown id.
    bool Insert(int id, float x, float y) {
        const int cell = CellIndex(x, y);
        if (cell < 0 || !Valid(id))
            return false;
        bucket(cell).push_back(id);
        cellOf_[static_cast<std::size_t>(id)] = cell;
        return true;
    }

    // ── MOVE IS THE OPERATION THE PATTERN GETS WRONG. When the entity stays in the
    // same cell -- the overwhelming majority of frames -- this does NO bookkeeping at
    // all: it is a lookup and a comparison. Only a CROSSING touches two buckets.
    // ️ RETURNS "DID IT CROSS", NOT "DID IT WORK". A valid call that stays inside
    // one bucket returns false -- but it is still counted by `Moves`, because that is
    // a call the partition answered for free.
    bool Move(int id, float x, float y) {
        const int cell = CellIndex(x, y);
        if (cell < 0 || !Valid(id))
            return false;
        ++moveCalls_;                     // accepted: an id in the grid, a real position
        const std::size_t index = static_cast<std::size_t>(id);
        if (cellOf_[index] == cell)
            return false;                 // same bucket: nothing to move
        if (cellOf_[index] >= 0)
            Detach(id, cellOf_[index]);   // ⚠️ leave the old bucket, or it is still found there
        bucket(cell).push_back(id);
        cellOf_[index] = cell;
        ++moves_;
        return true;
    }

    // Take an entity out of the grid entirely. Idempotent: removing what is not in
    // the grid is ignored, so it cannot detach a bucket entry twice.
    bool Remove(int id) {
        if (!Valid(id) || cellOf_[static_cast<std::size_t>(id)] < 0)
            return false;
        Detach(id, cellOf_[static_cast<std::size_t>(id)]);
        cellOf_[static_cast<std::size_t>(id)] = kNotInGrid;
        return true;
    }

    // The entities in ONE cell -- what a query "who is here" costs. Empty vector for
    // an out-of-world position, never a crash.
    //
    // ⚠️ THE REFERENCE IS INTO THE BUCKET, so any Insert, Move or Remove may invalidate
    // it -- a Move that swap-and-pops the entry you are holding is the one that bites.
    // Copy what you need, or use `CandidatesAround`, which fills a caller-owned vector.
    const std::vector<int> &CandidatesAt(float x, float y) const {
        const int cell = CellIndex(x, y);
        return cell < 0 ? empty() : buckets_[static_cast<std::size_t>(cell)];
    }

    // The entities in a (2*ring+1)^2 block of cells around the position -- the
    // chapter's melee query, where the unit's own cell is not enough because an
    // attacker standing one cell over still reaches. Appends into `out`.
    //
    // ⚠️ EVERY ENTITY IS IN EXACTLY ONE BUCKET, SO THE RESULT HAS NO DUPLICATES ON
    // ITS OWN. That is a property of the data structure (pinned by a spec), not
    // something this loop filters for -- which is exactly what makes a duplicate a
    // sign that the placement is broken rather than a thing to paper over.
    void CandidatesAround(float x, float y, int ring, std::vector<int> &out) const {
        out.clear();
        const int cell = CellIndex(x, y);
        if (cell < 0)
            return;
        const int cx = cell % cells_;
        const int cy = cell / cells_;
        for (int gy = cy - ring; gy <= cy + ring; ++gy) {
            for (int gx = cx - ring; gx <= cx + ring; ++gx) {
                if (gx < 0 || gy < 0 || gx >= cells_ || gy >= cells_)
                    continue;
                const std::vector<int> &b = buckets_[static_cast<std::size_t>(CellIndexFor(gx, gy))];
                out.insert(out.end(), b.begin(), b.end());
            }
        }
    }

    std::size_t BucketSize(int cell) const {
        if (cell < 0 || cell >= static_cast<int>(buckets_.size()))
            return 0;
        return buckets_[static_cast<std::size_t>(cell)].size();
    }

    // ⚠️ THE COUNTERS THAT SHOW THE PATTERN WORKING. `Crossings` counts accepted
    // `Move` calls that actually changed bucket; `Moves` counts every accepted call.
    // The gap is the work the partition did not do, and it is why a per-frame position
    // update is cheap.
    std::size_t Crossings() const { return moves_; }
    std::size_t Moves() const { return moveCalls_; }
    void ResetCounters() { moves_ = moveCalls_ = 0; }

private:
    static constexpr int kNotInGrid = -1;

    bool Valid(int id) const {
        return id >= 0 && id < static_cast<int>(cellOf_.size());
    }
    static bool InWorld(float v) { return v >= 0.0f && v < 1.0f; }

    int CellIndexFor(int cx, int cy) const { return cy * cells_ + cx; }

    std::vector<int> &bucket(int cell) { return buckets_[static_cast<std::size_t>(cell)]; }

    // ⚠️ SWAP-AND-POP, so a removal is O(1) and does not shift the bucket. The order
    // inside a bucket is therefore not stable, which nothing here depends on -- but a
    // caller that iterates a bucket while removing from it must not assume the
    // iteration order holds across the removal.
    void Detach(int id, int cell) {
        std::vector<int> &b = buckets_[static_cast<std::size_t>(cell)];
        for (std::size_t i = 0; i < b.size(); ++i) {
            if (b[i] == id) {
                b[i] = b.back();
                b.pop_back();
                return;
            }
        }
    }

    static const std::vector<int> &empty() {
        static const std::vector<int> none;
        return none;
    }

    int cells_;
    std::vector<std::vector<int>> buckets_;
    std::vector<int>              cellOf_;   // id -> cell, or kNotInGrid
    std::size_t                   moves_     = 0;   // crossings
    std::size_t                   moveCalls_ = 0;   // every Move that was accepted
};

} // namespace partition