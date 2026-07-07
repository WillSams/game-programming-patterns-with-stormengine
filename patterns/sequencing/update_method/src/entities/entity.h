#pragma once

// ── Update Method pattern ────────────────────────────────────────────────────
// (Game Programming Patterns, Robert Nystrom — Sequencing Patterns)
//
// Each game object gets an update() called once per frame; it advances that one
// object's behavior by a slice of time and keeps whatever state it needs between
// frames. A single loop over a collection of entities (see World) drives them
// all, so heterogeneous objects — a patrolling guard, a periodic turret — each
// mind their own behavior without one giant tangled update function.

// Namespaced to avoid colliding with Storm Engine v2's own ECS `Entity`.
namespace um {

struct Vec2 {
    float x = 0.f;
    float y = 0.f;
};

class Entity {
public:
    virtual ~Entity() = default;

    // Advance this entity's behavior by dt seconds.
    virtual void update(float dt) = 0;

    Vec2 position() const { return pos_; }

protected:
    Vec2 pos_;
};

} // namespace um
