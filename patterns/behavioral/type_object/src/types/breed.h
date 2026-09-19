#pragma once

#include <string>

// ── A breed: the TYPE OBJECT ─────────────────────────────────────────────────
//
// THE PATTERN IN ONE SENTENCE: instead of a class per kind of thing, have one
// class for all of them plus an object -- this one -- that holds what varies.
// A new breed is DATA, not a class: no header, no rebuild, no new `virtual`.
//
// The chapter's own framing is "the type object pattern", and its two payoffs are
// both visible here:
//
//   1. A BREED CAN BE BASED ON ANOTHER BREED. `parent` is why a dragon can be "a
//      brute, but with more health" without restating the brute's attack. That is
//      the half a plain data table does not give you.
//   2. MANY THINGS SHARE ONE TYPE. A hundred monsters point at one `Breed`, which
//      is the same data-sharing that makes this cheaper than class-per-kind --
//      and `specs/` pins it by address.
//
// ⚠️ UNSET IS A SENTINEL, DOCUMENTED AT THE FIELD, not a parallel `bool`. CODING.md
// tenet 11 prefers exactly this: "Prefer a sentinel documented at the field
// (`Injury::daysOut == 0` means available) over a parallel `bool` that can
// disagree with it." Here `kUnset` (-1) means "I did not say, ask my parent", and
// since health and attack are never legitimately negative there is no ambiguity.
//
// TWO LIMITS, documented rather than enforced, because a demo should name a
// decision instead of hiding it: a NEGATIVE value is "set" and passes through
// unchanged (the sentinel is -1 and nothing clamps it), and `Monster::heal` is
// UNCAPPED -- healing past the breed's health is allowed, which is a design choice
// a game with a max would make differently.
namespace types {

inline constexpr int kUnset = -1;

struct Breed {
    std::string name;
    int         health = kUnset;    // kUnset = inherit from `parent`
    int         attack = kUnset;    // kUnset = inherit from `parent`
    const Breed *parent = nullptr;  // null = nothing to inherit from

    // The value, or the parent's, or the parent's parent's... The walk is the
    // inheritance, and it is deliberately HERE rather than in a subclass: this is
    // the one place that decides what "unsaid" means.
    //
    // A breed with nothing set anywhere yields 0, which is documented rather than
    // accidental -- and `specs/` pins it, so it cannot drift into being special.
    //
    // ⚠️ THE WALK IS BOUNDED, AND THAT IS NOT DEFENSIVENESS FOR ITS OWN SAKE: a
    // CYCLE IN THE PARENT CHAIN HANGS. The shipped bestiary cannot cycle -- its
    // breeds are statics with fixed parents -- but `Breed` is a plain aggregate, so
    // a caller can write `a.parent = &b; b.parent = &a;` and an unbounded `for`
    // then spins forever with no output at all. A depth cap turns a hang into a
    // documented answer, and a spec pins a two-breed cycle terminating.
    static constexpr int kMaxDepth = 16;

    int healthValue() const {
        const Breed *b = this;
        for (int depth = 0; b && depth < kMaxDepth; ++depth, b = b->parent)
            if (b->health != kUnset)
                return b->health;
        return 0;
    }

    int attackValue() const {
        const Breed *b = this;
        for (int depth = 0; b && depth < kMaxDepth; ++depth, b = b->parent)
            if (b->attack != kUnset)
                return b->attack;
        return 0;
    }

    bool inheritsFrom(const Breed &other) const {
        for (const Breed *b = parent; b; b = b->parent)
            if (b == &other)
                return true;
        return false;
    }
};

} // namespace types
