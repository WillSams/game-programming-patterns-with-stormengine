#pragma once

#include "component.h"

// ── The parts ────────────────────────────────────────────────────────────────
//
// Three components, and every entity in the demo is a different COMBINATION of
// them. There is no `Player` class, no `Projectile` class and no `Prop` class --
// and adding a fourth part would add a fourth kind of thing to compose with,
// without touching any of these.
//
// ⚠️ NONE OF THEM KNOWS THE OTHERS EXIST. Input writes `vx`; Physics integrates
// `vx` into `x`; Appearance reads `x`. They agree about the TRANSFORM, which is
// the entity's, and about nothing else. That is why they can be reordered,
// omitted, or written by someone who has not read the other two.
namespace components {

// Turns intent into velocity. `jump` is only honoured on the ground, so the input
// component reads `y` -- the one place these parts touch shared state to decide
// something rather than to do their own job.
class InputComponent : public Component {
public:
    explicit InputComponent(float speed = 8.f, float jumpSpeed = 14.f)
        : speed_{speed}, jumpSpeed_{jumpSpeed} {}

    void update(Entity &e, const InputState &in) override {
        Transform &t = e.transform();
        t.vx = (in.right ? speed_ : 0.f) - (in.left ? speed_ : 0.f);
        if (in.jump && t.y <= 0.f)
            t.vy = jumpSpeed_;
    }

    const char *name() const override { return "INPUT"; }

private:
    float speed_;
    float jumpSpeed_;
};

// Integrates the velocity the input produced, and applies gravity. It does not
// care who set `vx`, or whether anyone did -- an entity with no input component
// simply keeps whatever velocity it has, which is a projectile.
class PhysicsComponent : public Component {
public:
    explicit PhysicsComponent(float gravity = 1.5f, float floor = 0.f)
        : gravity_{gravity}, floor_{floor} {}

    void update(Entity &e, const InputState &) override {
        Transform &t = e.transform();
        t.vy -= gravity_;
        t.x += t.vx;
        t.y += t.vy;
        if (t.y < floor_) {      // land, and stop falling
            t.y = floor_;
            t.vy = 0.f;
        }
    }

    const char *name() const override { return "PHYSICS"; }

private:
    float gravity_;
    float floor_;
};

// Describes where to draw the entity. It reads the transform AFTER whoever moves
// it, which is why it goes last -- see the ordering note in component.h.
class AppearanceComponent : public Component {
public:
    explicit AppearanceComponent(float size = 12.f) : size_{size} {}

    void update(Entity &e, const InputState &) override {
        const Transform &t = e.transform();
        e.draw({t.x, t.y, size_});
    }

    const char *name() const override { return "APPEARANCE"; }

private:
    float size_;
};

} // namespace components
