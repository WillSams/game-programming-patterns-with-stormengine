#pragma once

#include <cstddef>
#include <string>
#include <vector>

// ── Data Locality pattern: organize the DATA, not the loop ───────────────────
//
// The chapter's problem is not a slow algorithm -- it is that the algorithm spends
// its time WAITING. A loop that walks an array of pointers touches one object per
// cache line, so every element is a miss; the same loop over contiguous data walks
// the cache line by line and misses once per sixteen elements. Nothing about the
// arithmetic changes. The LAYOUT does.
//
// ⚠️ WHAT A SPEC CAN AND CANNOT PIN HERE, because this is the pattern where it
// matters most. A test cannot assert that memory is faster -- that measurement
// belongs to a machine, not to a test suite, and a timing assertion is a flake
// generator. So the specs pin the two things that ARE deterministic:
//
//   1. THE OPTIMIZATION DOES NOT CHANGE THE RESULT. The mixed layout and the split
//      layout must produce the same world, and a spec steps both and compares.
//   2. THE LAYOUT IS WHAT THE PATTERN CLAIMS. The hot struct is exactly its hot
//      fields, the elements are exactly one struct apart with no padding, and the
//      cold array is byte-identical after a hot update -- the last one being the
//      actual claim ("the hot loop did not drag cold data in"), checked rather
//      than asserted.
//
// The demo times the two. The specs prove the fast one is not a different world.
namespace locality {

// ── THE HOT FIELDS. These are what `update` touches, and nothing else belongs
// here: every byte in this struct is a byte pulled into cache for every element.
struct ParticleHot {
    float x, y;
    float vx, vy;
};

// ── THE COLD FIELDS. Read when drawing, and by nothing in the loop. Split out so
// that a walk over the hot array never pulls any of it in.
//
// ⚠️ NOT a "cold" struct that is small -- it is cold because the HOT LOOP never
// reads it. A struct full of std::string is exactly what should not be next to a
// float in the same cache line.
struct ParticleCold {
    std::string name;
    int         color;
    bool        visible;
};

// ── THE MIXED LAYOUT: what you write first, and the thing the pattern is about.
// One array of one struct holding hot fields and cold fields together, so each
// element's cache line is mostly data the loop will not read.
struct ParticleMixed {
    float       x, y;
    float       vx, vy;
    std::string name;
    int         color;
    bool        visible;
};

// An "object" that counts how often it was updated -- the spec's instrument for
// "every element, exactly once". It is deliberately a FIELD, so that the count
// lives in the same memory as the data being walked.
struct VisitCounter {
    int visits = 0;
};

// ── THE VIEW BOX: the bounds the world keeps its particles inside. A rectangle,
// because that is what it is -- a horizontal range with a vertical range implied
// somewhere else is how a drawing routine ends up inventing its own extent.
struct View {
    float minX = -50.0f;
    float maxX =  50.0f;
    float minY = -50.0f;
    float maxY =  50.0f;
};

// The ONE update rule, applied identically by both layouts. Two copies of this is
// how an "optimization" silently becomes a different simulation, so there is one.
inline void StepParticle(ParticleHot &p, float dt, const View &view) {
    p.x += p.vx * dt;
    p.y += p.vy * dt;
    // Bounce off the view edges -- a branch on the HOT data, which is the point:
    // the field the branch reads is in the cache line that was already fetched.
    if (p.x < view.minX) { p.x = view.minX; p.vx = -p.vx; }
    if (p.x > view.maxX) { p.x = view.maxX; p.vx = -p.vx; }
    if (p.y < view.minY) { p.y = view.minY; p.vy = -p.vy; }
    if (p.y > view.maxY) { p.y = view.maxY; p.vy = -p.vy; }
}

// ── LAYOUT A: one array of mixed structs. Walk it and step each element.
class MixedWorld {
public:
    explicit MixedWorld(std::size_t count) { particles_.resize(count, ParticleMixed{}); }

    void Set(std::size_t i, float x, float y, float vx, float vy, const std::string &name) {
        particles_[i].x = x;
        particles_[i].y = y;
        particles_[i].vx = vx;
        particles_[i].vy = vy;
        particles_[i].name = name;
    }

    void Step(float dt, const View &view) {
        for (ParticleMixed &m : particles_) {
            ParticleHot hot{m.x, m.y, m.vx, m.vy};   // pulled OUT of a large struct
            StepParticle(hot, dt, view);
            m.x = hot.x; m.y = hot.y; m.vx = hot.vx; m.vy = hot.vy;
        }
    }

    std::size_t size() const { return particles_.size(); }
    const ParticleMixed &at(std::size_t i) const { return particles_[i]; }
    ParticleMixed &at(std::size_t i) { return particles_[i]; }

private:
    std::vector<ParticleMixed> particles_;
};

// ── LAYOUT B: the hot array and the cold array, side by side, indexed the same.
// Walking `hot_` touches nothing else.
class SplitWorld {
public:
    explicit SplitWorld(std::size_t count) {
        hot_.resize(count, ParticleHot{});
        cold_.resize(count, ParticleCold{});
        visits_.resize(count, VisitCounter{});
    }

    void Set(std::size_t i, float x, float y, float vx, float vy, const std::string &name) {
        hot_[i].x = x;
        hot_[i].y = y;
        hot_[i].vx = vx;
        hot_[i].vy = vy;
        cold_[i].name = name;
        cold_[i].color = 0xFFFFFF;
        cold_[i].visible = true;
    }

    // ⚠️ THE HOT LOOP. It reads and writes `hot_` and `visits_` and does not touch
    // `cold_` at all -- which is then checked rather than trusted.
    void Step(float dt, const View &view) {
        for (std::size_t i = 0; i < hot_.size(); ++i) {
            StepParticle(hot_[i], dt, view);
            ++visits_[i].visits;
        }
    }

    std::size_t size() const { return hot_.size(); }
    const ParticleHot &hotAt(std::size_t i) const { return hot_[i]; }
    ParticleHot &hotAt(std::size_t i) { return hot_[i]; }
    const ParticleCold &coldAt(std::size_t i) const { return cold_[i]; }
    // ⚠️ COLD DATA GETS ITS OWN MUTATOR, AND THAT IS THE POINT. There is no
    // non-const `coldAt`, so the only way to change a cold field is a named method
    // here -- which is what keeps the hot loop from acquiring a stray write to the
    // cold side. The spec that flips a cold flag exists because that write would
    // otherwise be invisible: nothing would fail, the split would simply stop
    // buying anything.
    void SetVisible(std::size_t i, bool visible) { cold_[i].visible = visible; }
    int visitsAt(std::size_t i) const { return visits_[i].visits; }

    // For the spec that checks the loop touched exactly the hot side.
    const unsigned char *ColdBytes() const {
        return reinterpret_cast<const unsigned char *>(cold_.data());
    }
    std::size_t ColdBytesSize() const {
        return cold_.size() * sizeof(ParticleCold);
    }

private:
    std::vector<ParticleHot>   hot_;
    std::vector<ParticleCold>  cold_;
    std::vector<VisitCounter>  visits_;
};

// ── THE CONTRACT BETWEEN THE TWO: same world, layout only. Compared with a
// tolerance rather than exactly, and that is deliberate: both loops perform the
// same operations in the same order, but a compiler is free to vectorize one and
// contract a multiply-add in the other, so the last bit may differ. Insisting on
// exact equality would make this spec fail for a legitimate optimization -- a
// flake that blames the code for the compiler's arithmetic.
inline bool SameWorld(const MixedWorld &a, const SplitWorld &b, float tolerance) {
    if (a.size() != b.size())
        return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        const float dx = a.at(i).x - b.hotAt(i).x;
        const float dy = a.at(i).y - b.hotAt(i).y;
        const float d = (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);
        if (d > tolerance)
            return false;
    }
    return true;
}

// The layouts are NOT interchangeable without a mapping, and saying so is part of
// the pattern: a split world's cold data has to be reunited with the hot data by a
// shared index. That is the cost of the optimization.
inline void CopyHotFromSplit(const SplitWorld &from, std::size_t i, MixedWorld &to) {
    to.at(i).x = from.hotAt(i).x;
    to.at(i).y = from.hotAt(i).y;
    to.at(i).vx = from.hotAt(i).vx;
    to.at(i).vy = from.hotAt(i).vy;
}

} // namespace locality
