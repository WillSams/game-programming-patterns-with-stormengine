#include <igloo/igloo_alt.h>

#include "../src/types/bestiary.h"
#include "../src/types/monster.h"

using namespace igloo;
using namespace types;

// The pattern's core, spec'd with no window. Two things are being pinned: what a
// type object ANSWERS (including through its parents), and that instances SHARE it
// rather than copying from it.
Describe(TypeObjectSpec) {

  // A base with everything set, and a child that sets one field and inherits the
  // other -- the shape the whole pattern exists for.
  static const Breed &Base() { return kCritter; }

  Describe(TheBreedAnswersForItself) {
    It(returns_the_values_it_states) {
      Assert::That(Base().healthValue(), Equals(20));
      Assert::That(Base().attackValue(), Equals(3));
      Assert::That(Base().name, Equals("CRITTER"));
    };

    It(has_no_parent_to_inherit_from) {
      Assert::That(Base().parent == nullptr, IsTrue());
    };

    // The documented default, pinned so it cannot drift into being special: a
    // breed with nothing set anywhere answers 0, not a garbage read and not -1.
    It(answers_zero_when_neither_it_nor_any_parent_says) {
      const Breed nothing{"NOTHING"};
      Assert::That(nothing.healthValue(), Equals(0));
      Assert::That(nothing.attackValue(), Equals(0));
    };
  };

  Describe(AChildInheritsWhatItDoesNotState) {
    It(takes_the_field_it_left_unset_from_its_parent) {
      Assert::That(kBrute.attack, Equals(12));           // it said so
      Assert::That(kBrute.health, Equals(kUnset));        // it did not
      Assert::That(kBrute.healthValue(), Equals(20));     // the critter's
      Assert::That(kBrute.attackValue(), Equals(12));
    };

    It(overrides_the_field_its_parent_had_set) {
      Assert::That(kDragon.attackValue(), Equals(12));    // the brute's, still
      Assert::That(kDragon.healthValue(), Equals(100));   // its own
    };

    // THE WALK, which is the half a flat data table does not give you: the dragon
    // inherits `attack` from its GRANDPARENT, having never named it.
    It(walks_more_than_one_level_of_parents) {
      Assert::That(kDragon.inheritsFrom(kBrute), IsTrue());
      Assert::That(kDragon.inheritsFrom(kCritter), IsTrue());
      Assert::That(kDragon.inheritsFrom(kDragon), IsFalse());   // not its own parent
      Assert::That(kCritter.inheritsFrom(kBrute), IsFalse());   // nor upward
    };
  };

  // The other half: instances hold the type object, they do not copy it.
  Describe(InstancesShareTheTypeObject) {
    It(takes_its_stats_from_the_breed) {
      Monster m{kDragon, 1};
      Assert::That(m.health(), Equals(100));    // its own, seeded from the breed
      Assert::That(m.attack(), Equals(12));     // the breed's, every time
      Assert::That(m.name(), Equals("DRAGON"));
    };

    // BY ADDRESS, because a copy would satisfy every value assertion above and
    // still not be the pattern. This is the one that would fail if `Monster` held
    // a `Breed` by value.
    It(points_at_the_same_breed_for_every_monster_of_it) {
      Monster a{kBrute, 1};
      Monster b{kBrute, 2};
      Assert::That(&a.breed() == &b.breed(), IsTrue());
      Assert::That(&a.breed() == &kBrute, IsTrue());
    };

    // Per-instance state stays per-instance: hurting one brute must not hurt the
    // next, and must not touch the shared type.
    It(keeps_its_own_health_to_itself) {
      Monster a{kBrute, 1};
      Monster b{kBrute, 2};
      a.takeDamage(7);
      Assert::That(a.health(), Equals(13));
      Assert::That(b.health(), Equals(20));
      Assert::That(kBrute.healthValue(), Equals(20));
    };
  };

  Describe(CombatIsPerInstance) {
    It(loses_health_to_damage) {
      Monster m{kCritter, 1};
      m.takeDamage(5);
      Assert::That(m.health(), Equals(15));
      Assert::That(m.alive(), IsTrue());
    };

    It(does_not_go_below_zero) {
      Monster m{kCritter, 1};
      m.takeDamage(999);
      Assert::That(m.health(), Equals(0));
      Assert::That(m.alive(), IsFalse());
    };

    It(heals_back_up) {
      Monster m{kCritter, 1};
      m.takeDamage(999);
      m.heal(4);
      Assert::That(m.health(), Equals(4));
      Assert::That(m.alive(), IsTrue());
    };

    It(describes_itself_from_both_its_breed_and_its_own_state) {
      Monster m{kDragon, 7};
      Assert::That(m.describe(), Equals("DRAGON #7 HP 100 ATK 12"));
    };
  };

  // ⚠️ A CYCLE WOULD HANG AN UNBOUNDED WALK. The bestiary cannot produce one --
  // its breeds are statics with fixed parents -- but `Breed` is a plain aggregate,
  // so a caller can. This case would HANG (not fail) without the depth cap, which
  // makes it the one spec here that guards a hang rather than a wrong answer.
  Describe(AcyclicParentChainsAreSurvivable) {
    It(terminates_on_a_two_breed_cycle_instead_of_hanging) {
      Breed a{"A"};
      Breed b{"B"};
      a.parent = &b;
      b.parent = &a;
      Assert::That(a.healthValue(), Equals(0));    // nothing set anywhere
      Assert::That(b.attackValue(), Equals(0));
    };

    It(terminates_on_a_cycle_that_has_a_value_in_it) {
      Breed a{"A"};
      Breed b{"B"};
      a.parent = &b;
      b.parent = &a;
      a.health = 5;
      Assert::That(a.healthValue(), Equals(5));
      Assert::That(b.healthValue(), Equals(5));    // one step round to a's value
    };
  };

  // ⚠️ THE CLAIM THE PATTERN MAKES, TESTED AS A CLAIM: a new kind of thing is a new
  // VALUE, not a new class. This breed is declared right here, in a spec, with no
  // header, no subclass and no change to `Monster` -- and it inherits, overrides
  // and behaves like the shipped ones because nothing about it is special.
  Describe(AddingABreedIsAddingData) {
    It(needs_no_class_and_still_inherits_and_overrides) {
      static const Breed kind{"SPECTRE", kUnset, 40, &kDragon};
      Assert::That(kind.healthValue(), Equals(100));   // the dragon's, two levels up
      Assert::That(kind.attackValue(), Equals(40));    // its own
      Monster m{kind, 1};
      Assert::That(m.name(), Equals("SPECTRE"));
      Assert::That(m.describe(), Equals("SPECTRE #1 HP 100 ATK 40"));
    };
  };
};
