#pragma once

#include <string>

#include "sandbox.h"

// ── The base class, and the two things it provides ───────────────────────────
//
// SUBCLASS SANDBOX: the base offers a set of operations -- a "sandbox" -- and
// every subclass implements ONE method by composing them. Two payoffs, and both
// are visible in this file:
//
//   1. THE VOCABULARY IS DELIBERATE. A subclass can say `move`, `playSound`,
//      `spawnParticles`, `addHealth`, `setHealth` and nothing else. It cannot
//      reach the world, the renderer or the audio system, because it is never
//      given them -- only `Sandbox`. The chapter calls this the pattern's real
//      value: subclasses are powerful and CONTAINED.
//   2. SHARED WORK IS HOISTED, NOT COPIED. `jumpTo` below is built from the
//      vocabulary, so every power that jumps jumps the same way. Three copies of
//      a jump in three subclasses would be three things to keep in step.
//
// ⚠️ NVI: `activate` IS PUBLIC AND NON-VIRTUAL, and that is what makes point 1
// structural rather than a convention. The subclass overrides `onActivate`, which
// has no world parameter at all -- so the only route to the sandbox is the
// protected vocabulary in this class. A public `virtual activate(Sandbox&)`
// would have let any subclass ignore the sandbox and take whatever it liked.
namespace powers {

class Superpower {
public:
    virtual ~Superpower() = default;

    // What the power is called, for the demo's list.
    virtual const char *name() const = 0;

    // The one entry point. Not virtual, and not overridable.
    void activate(Sandbox &world) {
        world_ = &world;
        onActivate();
        world_ = nullptr;
    }

protected:
    // The subclass's entire job.
    virtual void onActivate() = 0;

    // ── the vocabulary ──────────────────────────────────────────────────────
    void move(int dx, int dy) { world_->move(dx, dy); }
    void playSound(const std::string &name) { world_->playSound(name); }
    void spawnParticles(int count) { world_->spawnParticles(count); }
    void addHealth(int amount) { world_->addHealth(amount); }
    void setHealth(int value) { world_->setHealth(value); }
    int  health() const { return world_->health(); }

    // ── a shared operation, built FROM the vocabulary ───────────────────────
    // This is the half that removes duplication: a jump is one implementation,
    // and it is written in the same words the subclasses use.
    //
    // It moves RELATIVE to where the world already is, which is what makes it a
    // helper rather than another operation: `move` is the primitive, `jumpTo` is
    // a sentence composed from it.
    void jumpTo(int targetX, int targetY) {
        playSound("jump");
        spawnParticles(8);
        move(targetX - world_->x(), targetY - world_->y());
    }

private:
    Sandbox *world_ = nullptr;
};

} // namespace powers
