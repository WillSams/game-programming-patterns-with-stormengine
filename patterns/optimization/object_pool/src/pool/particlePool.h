#pragma once

#include <cstddef>
#include <vector>

// ── Object Pool pattern: pay for the objects once, then reuse them ────────────
//
// The chapter's problem is that `new` and `delete` are a poor fit for things that
// are created and destroyed constantly -- particles, bullets, sound events. Each
// one is a trip to the allocator, a trip to the free store, and a fragment the
// heap keeps for the rest of the run. A POOL allocates the whole set up front and
// hands out the objects that are not currently in use.
//
// The two ideas worth the reader's time are here in the implementation:
//
//   * THE FREE LIST LIVES INSIDE THE OBJECTS. A slot that is not in use stores the
//     index of the next free slot in its own `nextFree_`, so the pool needs no
//     side array, no allocation, and no bookkeeping beyond one integer head. That
//     is the trick the chapter spends its sample code on, and it is why taking and
//     returning an object are both O(1).
//
//   * EXHAUSTION IS A NORMAL ANSWER, NOT AN ERROR. A fixed pool runs out. `Create`
//     says so by returning -1, and the caller decides whether to drop the spawn or
//     to size the pool differently. What it must never do is quietly overwrite a
//     live object, or grow. Both are checked by the specs.
//
// ⚠️ AND THE PATTERN'S REAL DANGER IS THAT REUSE IS INVISIBLE. A returned object
// still looks exactly like the object that was there before, at the same address.
// Code that kept a POINTER into the pool across a frame can find it pointing at a
// different particle with no warning -- the classic stale-reference bug that object
// pools are famous for. This implementation hands out an INDEX and makes the caller
// ask `IsAlive` before trusting it, which at least makes the mistake a visible one.
// It does not hand out generation counters; a demo that never holds a handle across
// frames would not use them, and unused machinery is the thing this repo avoids.
namespace pool {

// The particle the chapter pools. Small and trivially copyable on purpose: a pool
// of large objects saves more, and a pool of objects with real destructors needs
// `Animate` to run them -- see the note on `Released`.
struct Particle {
    float x     = 0.0f;
    float y     = 0.0f;
    float vx    = 0.0f;
    float vy    = 0.0f;
    float life  = 0.0f;      // seconds remaining
    float maxLife = 1.0f;    // what `life` started at, so alpha can be derived
    bool  alive = false;

    // 1.0 at spawn, 0.0 at expiry. Deliberately computed rather than stored: it is
    // a property of two fields that already exist.
    float Alpha() const { return maxLife > 0.0f ? life / maxLife : 0.0f; }
};

class ParticlePool {
public:
    // ⚠️ THE CAPACITY IS THE WHOLE COMMITMENT. The vector is sized once, here, and
    // is never resized again -- not on exhaustion, not on Clear. Everything below
    // assumes `slots_.size()` does not move.
    explicit ParticlePool(std::size_t capacity) : slots_(capacity) {
        ResetFreeList();
    }

    // ── TAKE ONE. Returns the slot's index, or -1 when the pool is exhausted.
    // O(1): pop the free-list head and write over the old contents.
    int Create(float x, float y, float vx, float vy, float life) {
        // ⚠️ A PARTICLE WITH NO LIFE IS A CALLER BUG, AND IT IS REFUSED HERE RATHER
        // THAN STORED. Accepting it would make a slot that is `alive` with `life <= 0`
        // and `Alpha()` negative -- a live object that is already expired, which only
        // `Animate` would clean up, and only if the caller ever calls it. Not counted
        // in `Refused`, because that counter answers "is the pool too small".
        if (life <= 0.0f)
            return -1;
        if (freeHead_ < 0) {
            ++refused_;              // refused, NOT overwritten -- the live pool stands
            return -1;
        }
        const int index = freeHead_;
        Slot &slot = slots_[static_cast<std::size_t>(index)];
        freeHead_ = slot.nextFree;   // the free list was stored in the slot itself

        ++created_;
        if (slot.usedBefore)
            ++reused_;               // this slot had a previous life
        else
            slot.usedBefore = true;
        Particle &p = slot.particle;
        p.x = x; p.y = y; p.vx = vx; p.vy = vy;
        p.life = life; p.maxLife = life > 0.0f ? life : 1.0f;
        p.alive = true;
        ++alive_;
        return index;
    }

    // ── ADVANCE EVERY LIVE PARTICLE, and free the ones whose life ran out.
    //
    // ⚠️ THE DEAD ARE SKIPPED, NOT REMOVED. There is no erase, no compaction and no
    // `alive` array to shift -- the iteration is over all slots every time, and a
    // dead slot costs one branch. That is the trade a pool makes: the per-frame walk
    // is fixed and predictable, and it is paid at the capacity you chose up front.
    void Animate(float dt) {
        for (std::size_t i = 0; i < slots_.size(); ++i) {
            Particle &p = slots_[i].particle;
            if (!p.alive)
                continue;
            p.x += p.vx * dt;
            p.y += p.vy * dt;
            p.life -= dt;
            if (p.life <= 0.0f) {
                p.life = 0.0f;
                Release(static_cast<int>(i));
            }
        }
    }

    // Hand a slot back. Idempotent per slot from the pool's point of view: releasing
    // a dead slot is ignored, so a double release cannot corrupt the free list by
    // putting one slot on it twice.
    void Release(int index) {
        if (!Valid(index))
            return;
        Slot &slot = slots_[static_cast<std::size_t>(index)];
        if (!slot.particle.alive)
            return;                  // already free; a second release would double-link
        slot.particle.alive = false;
        --alive_;
        ++released_;
        slot.nextFree = freeHead_;   // push onto the free list
        freeHead_ = index;
    }

    // Everything back to free, in one step. Same capacity -- a client that wants a
    // different size constructs a different pool, because `slots_` never resizes.
    //
    // ⚠️ `usedBefore` IS DELIBERATELY KEPT. A cleared slot has still had a life, so
    // the next take of it is a reuse, and `Reused` must not fall back to zero just
    // because the pool was emptied. Clear is about what is alive, not about history.
    void Clear() {
        for (std::size_t i = 0; i < slots_.size(); ++i)
            slots_[i].particle.alive = false;
        alive_ = 0;
        ResetFreeList();
    }

    std::size_t Capacity() const { return slots_.size(); }
    std::size_t AliveCount() const { return alive_; }
    std::size_t FreeCount() const { return slots_.size() - alive_; }

    bool IsAlive(int index) const {
        return Valid(index) && slots_[static_cast<std::size_t>(index)].particle.alive;
    }

    // ⚠️ ONLY MEANINGFUL FOR A LIVE SLOT. A dead slot still holds the last values it
    // had, which is the reuse trap in its purest form -- the bytes are there and
    // look plausible. `IsAlive` first.
    const Particle &At(int index) const {
        return slots_[static_cast<std::size_t>(index)].particle;
    }

    // ⚠️ NON-CONST, AND NARROW, FOR THE ONE THING THE POOL DOES NOT OWN: the
    // caller's physics. This demo adds gravity to `vy` through here and lets
    // `Animate` do the integration -- the pool owns life and reuse, the caller owns
    // acceleration, and a pool that hard-coded gravity would be a particle system
    // wearing a pool's name. `IsAlive` first: a dead slot holds the last particle's
    // bytes, which is the reuse trap this whole class documents.
    Particle &At(int index) {
        return slots_[static_cast<std::size_t>(index)].particle;
    }

    // ── The counters, because "was anything actually reused" is the question the
    // demo exists to answer and it is not visible in a screenshot.
    std::size_t Created() const { return created_; }   // successful takes, ever
    std::size_t Reused() const { return reused_; }     // ...of a slot used before
    std::size_t Refused() const { return refused_; }   // takes that hit a full pool
    std::size_t Released() const { return released_; }  // returned to the pool
    void ResetCounters() { created_ = reused_ = refused_ = released_ = 0; }

private:
    struct Slot {
        Particle particle;
        int  nextFree    = -1;    // ⚠️ THE FREE LIST, stored in the object it frees
        bool usedBefore  = false; // has this slot ever held a particle?
    };

    bool Valid(int index) const {
        return index >= 0 && index < static_cast<int>(slots_.size());
    }

    // Chains every slot into the free list, head first. Called once at construction
    // and again by `Clear`, so the two can never disagree about what "free" means.
    void ResetFreeList() {
        freeHead_ = slots_.empty() ? -1 : 0;
        for (std::size_t i = 0; i < slots_.size(); ++i)
            slots_[i].nextFree = (i + 1 < slots_.size())
                               ? static_cast<int>(i + 1) : -1;
    }

    std::vector<Slot> slots_;
    int               freeHead_ = -1;
    std::size_t       alive_    = 0;

    std::size_t created_ = 0;
    std::size_t reused_  = 0;
    std::size_t refused_ = 0;
    std::size_t released_ = 0;
};

} // namespace pool