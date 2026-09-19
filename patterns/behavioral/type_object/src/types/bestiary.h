#pragma once

#include "breed.h"

// ── The breeds, as DATA ──────────────────────────────────────────────────────
//
// ⚠️ THE PARENT CHAIN IS THE POINT, and it is the half a flat data table does not
// give you. Each of these states only what it CHANGES and inherits the rest:
//
//     CRITTER   hp 20, atk 3          -- the base
//     BRUTE     atk 12                -- "a critter that hits harder"
//     DRAGON    hp 100                -- "a brute that also has more health"
//
// A dragon therefore has hp 100 and atk 12 -- with `atk` never written down twice,
// and no class anywhere. Adding a fourth breed is adding a value here.
//
// THEY ARE STATIC AND THAT IS A LIFETIME RULE, not style: a Monster holds a
// reference to its breed, so a breed must outlive its monsters (see monster.h).
namespace types {

inline const Breed kCritter{"CRITTER", 20, 3, nullptr};
inline const Breed kBrute{"BRUTE", types::kUnset, 12, &kCritter};
inline const Breed kDragon{"DRAGON", 100, types::kUnset, &kBrute};

} // namespace types
