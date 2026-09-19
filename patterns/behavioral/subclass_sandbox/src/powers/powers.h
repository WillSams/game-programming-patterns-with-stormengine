#pragma once

#include "sandbox.h"
#include "superpower.h"

// ── The powers ───────────────────────────────────────────────────────────────
//
// Each is a NAME and a short composition of the base class's vocabulary. That is
// the whole of a subclass under this pattern: no member state, no constructor, no
// resource of its own -- just `onActivate`.
//
// ⚠️ THE SHARED HELPER IS THE POINT OF `SkyLaunch`. It and `SuperJump` both jump,
// and neither has a line of jump code: `jumpTo` lives in the base, written in the
// same vocabulary, so the two cannot drift apart. A pattern that hoists nothing
// would be buying containment at the price of duplication, which is a bad trade;
// the chapter is explicit that both halves matter.
namespace powers {

class SkyLaunch : public Superpower {
public:
    const char *name() const override { return "SKY LAUNCH"; }

protected:
    void onActivate() override {
        addHealth(10);
        spawnParticles(20);
        jumpTo(0, -40);        // launches upward
    }
};

class GroundDive : public Superpower {
public:
    const char *name() const override { return "GROUND DIVE"; }

protected:
    void onActivate() override {
        playSound("dive");
        spawnParticles(12);
        move(0, 60);           // dives downward
    }
};

class SuperJump : public Superpower {
public:
    const char *name() const override { return "SUPER JUMP"; }

protected:
    void onActivate() override {
        jumpTo(0, -80);        // the SAME helper as SkyLaunch, a different height
    }
};

class Fireball : public Superpower {
public:
    const char *name() const override { return "FIREBALL"; }

protected:
    void onActivate() override {
        setHealth(health() - 5);   // a power that costs something
        playSound("fireball");
        spawnParticles(30);
        move(0, -5);
    }
};

} // namespace powers
