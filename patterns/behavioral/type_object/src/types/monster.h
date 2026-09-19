#pragma once

#include <string>

#include "breed.h"

// ── The one class ────────────────────────────────────────────────────────────
//
// Every kind of creature is a `Monster`. What varies between kinds -- and there
// is no class for any of it -- lives in the `Breed` this holds.
//
// ⚠️ IT HOLDS A REFERENCE, AND THE REFERENCE IS THE PATTERN. A monster does not
// copy its breed's numbers: it points at the shared type object, so a hundred
// monsters of one breed cost one breed. The lifetime rule that buys is real and
// stated rather than implied: **the breed must outlive the monsters that name
// it**, which is why the bestiary (bestiary.h) is a set of static objects and not
// something a caller builds on the stack.
//
// A monster owns only what is *its*: its identity and its current health. That is
// the split the pattern asks for -- shared versus per-instance -- made visible.
namespace types {

class Monster {
public:
    Monster(const Breed &breed, int id) : breed_{&breed}, id_{id},
                                          health_{breed.healthValue()} {}

    const Breed &breed() const { return *breed_; }
    int          id() const { return id_; }

    // From the type object, not from this instance.
    int attack() const { return breed_->attackValue(); }
    std::string name() const { return breed_->name; }

    // This instance's own state.
    int  health() const { return health_; }
    bool alive() const { return health_ > 0; }
    void takeDamage(int amount) {
        health_ -= amount;
        if (health_ < 0)
            health_ = 0;
    }
    void heal(int amount) { health_ += amount; }

    // For the demo and for readable spec failures. One implementation, used by
    // both -- a spec that built its own string would be asserting a second
    // spelling of the same fact.
    std::string describe() const {
        return name() + " #" + std::to_string(id_) + " HP " +
               std::to_string(health_) + " ATK " + std::to_string(attack());
    }

private:
    const Breed *breed_;
    int          id_;
    int          health_;
};

} // namespace types
