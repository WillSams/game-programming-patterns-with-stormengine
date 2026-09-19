#include <igloo/igloo_alt.h>

#include "../src/powers/powers.h"

using namespace igloo;
using namespace powers;

// The pattern's core, spec'd without a window. Every case is about the SANDBOX:
// what the vocabulary records, and what each power composes from it.
//
// The expected strings come from DescribeEffects, which is the same function the
// demo prints with -- so these specs and that screen cannot disagree about what a
// power did.
Describe(SubclassSandboxSpec) {

  static Sandbox World() { return Sandbox{}; }

  // The vocabulary a base class chooses, and the only thing a subclass may say.
  Describe(TheSandboxVocabulary) {
    It(records_a_move_with_its_delta_and_applies_it) {
      Sandbox w = World();
      w.move(3, -7);
      Assert::That(DescribeEffects(w.log()), Equals("MOVE 3 -7"));
      Assert::That(w.x(), Equals(3));
      Assert::That(w.y(), Equals(-7));
    };

    It(records_a_sound_by_name) {
      Sandbox w = World();
      w.playSound("jump");
      Assert::That(DescribeEffects(w.log()), Equals("SOUND jump"));
    };

    It(records_a_particle_count) {
      Sandbox w = World();
      w.spawnParticles(20);
      Assert::That(DescribeEffects(w.log()), Equals("PARTICLES 20"));
    };

    It(adds_health_and_records_the_amount) {
      Sandbox w = World();
      w.addHealth(10);
      Assert::That(w.health(), Equals(110));   // starts at 100
      Assert::That(DescribeEffects(w.log()), Equals("ADD_HP 10"));
    };

    It(replaces_health_and_records_the_value) {
      Sandbox w = World();
      w.setHealth(7);
      Assert::That(w.health(), Equals(7));
      Assert::That(DescribeEffects(w.log()), Equals("SET_HP 7"));
    };

    It(resets_to_a_clean_world) {
      Sandbox w = World();
      w.move(5, 5);
      w.addHealth(-50);
      w.reset();
      Assert::That(w.log().empty(), IsTrue());
      Assert::That(w.x(), Equals(0));
      Assert::That(w.y(), Equals(0));
      Assert::That(w.health(), Equals(100));
    };
  };

  // Each power is a name and a composition of the base's vocabulary. These are
  // the exact sequences, in order -- the order is part of the power.
  Describe(PowersComposeTheVocabulary) {
    It(sky_launch_heals_spawns_and_jumps_up) {
      Sandbox w = World();
      SkyLaunch power;
      power.activate(w);
      Assert::That(DescribeEffects(w.log()),
                   Equals("ADD_HP 10 | PARTICLES 20 | SOUND jump | PARTICLES 8 | MOVE 0 -40"));
      Assert::That(w.health(), Equals(110));
      Assert::That(w.y(), Equals(-40));
    };

    It(ground_dive_dives_downward) {
      Sandbox w = World();
      GroundDive power;
      power.activate(w);
      Assert::That(DescribeEffects(w.log()),
                   Equals("SOUND dive | PARTICLES 12 | MOVE 0 60"));
      Assert::That(w.y(), Equals(60));
    };

    It(super_jump_jumps_higher_without_any_jump_code_of_its_own) {
      Sandbox w = World();
      SuperJump power;
      power.activate(w);
      Assert::That(DescribeEffects(w.log()),
                   Equals("SOUND jump | PARTICLES 8 | MOVE 0 -80"));
    };

    It(fireball_costs_health) {
      Sandbox w = World();
      Fireball power;
      power.activate(w);
      Assert::That(DescribeEffects(w.log()),
                   Equals("SET_HP 95 | SOUND fireball | PARTICLES 30 | MOVE 0 -5"));
      Assert::That(w.health(), Equals(95));
    };
  };

  // The half that stops the pattern being containment at the price of
  // duplication: a jump is written ONCE, in the base, in the same vocabulary.
  Describe(TheSharedHelper) {
    It(gives_both_jumping_powers_the_same_jump_shape) {
      // ⚠️ THE FIRST VERSION OF THIS CASE WAS WRONG, not the code: it compared the
      // two logs' LAST effects, which are the Move -- and the two moves differ BY
      // DESIGN, because SkyLaunch and SuperJump jump to different heights. What
      // the shared helper actually guarantees is the WORDS: a "jump" sound and 8
      // particles, in the same order. The height is the power's own business.
      Sandbox a = World();
      Sandbox b = World();
      SkyLaunch sky;
      SuperJump jump;
      sky.activate(a);
      jump.activate(b);

      Assert::That(DescribeEffects(b.log()),
                   Equals("SOUND jump | PARTICLES 8 | MOVE 0 -80"));

      const std::vector<Effect> &la = a.log();
      const std::vector<Effect> &lb = b.log();
      Assert::That(la.size(), Equals(static_cast<std::size_t>(5)));   // + heal, + spawn
      Assert::That(la[2] == lb[0], IsTrue());   // SOUND jump
      Assert::That(la[3] == lb[1], IsTrue());   // PARTICLES 8
      Assert::That(la[4].kind == lb[2].kind, IsTrue());   // both end by moving
      Assert::That(la[4].b != lb[2].b, IsTrue());         // to DIFFERENT heights
    };

    // A helper built from `move` moves RELATIVE to where the world is, which is
    // what makes it a sentence in the vocabulary rather than another primitive.
    It(moves_relative_to_where_the_world_already_is) {
      Sandbox w = World();
      w.move(10, 20);
      SuperJump jump;
      jump.activate(w);
      Assert::That(DescribeEffects(w.log()),
                   Equals("MOVE 10 20 | SOUND jump | PARTICLES 8 | MOVE -10 -100"));
      Assert::That(w.x(), Equals(0));
      Assert::That(w.y(), Equals(-80));   // the TARGET, not the delta
    };
  };

  Describe(TheConstraint) {
    It(does_nothing_until_a_power_is_activated) {
      Sandbox w = World();
      Assert::That(w.log().empty(), IsTrue());
      Assert::That(w.health(), Equals(100));
    };

    // No idempotency is promised, and saying so is better than assuming it: a
    // power activated twice does its thing twice.
    It(accumulates_effects_across_activations) {
      Sandbox w = World();
      GroundDive power;
      power.activate(w);
      power.activate(w);
      Assert::That(w.log().size(), Equals(static_cast<std::size_t>(6)));
      Assert::That(w.y(), Equals(120));
    };
  };

  // An effect table with a hole in it fails silently, exactly as the font's does.
  Describe(EffectCoverage) {
    It(names_every_effect_kind) {
      const EffectKind all[] = {EffectKind::Move, EffectKind::Sound,
                                EffectKind::Particles, EffectKind::AddHealth,
                                EffectKind::SetHealth};
      for (EffectKind k : all) {
        const char *n = EffectName(k);
        Assert::That(std::string(n) == "?", IsFalse());
        Assert::That(std::string(n).empty(), IsFalse());
      }
    };
  };
};
