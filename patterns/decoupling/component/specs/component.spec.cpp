#include <igloo/igloo_alt.h>

#include "../src/components/components.h"

using namespace igloo;
using namespace components;

// The pattern's core, spec'd with no window and no SDL event: an entity is a
// container, and what it does is what it holds.
Describe(ComponentSpec) {

  // ── builders, so each case says what it is about and not how to wire it ──
  static std::unique_ptr<Entity> Player() {
    auto e = std::make_unique<Entity>("PLAYER");
    e->add(std::make_unique<InputComponent>());
    e->add(std::make_unique<PhysicsComponent>());
    e->add(std::make_unique<AppearanceComponent>());
    return e;
  }
  static std::unique_ptr<Entity> Projectile() {
    auto e = std::make_unique<Entity>("PROJECTILE");
    e->transform().y = 20.f;
    e->add(std::make_unique<PhysicsComponent>());
    e->add(std::make_unique<AppearanceComponent>());
    return e;
  }
  static std::unique_ptr<Entity> Prop() {
    auto e = std::make_unique<Entity>("PROP");
    e->add(std::make_unique<AppearanceComponent>());
    return e;
  }

  Describe(AnEntityDoesOnlyWhatItIsMadeOf) {
    It(does_nothing_at_all_with_no_components) {
      Entity e{"BARE"};
      e.update(InputState{});
      Assert::That(e.transform().x, Equals(0.f));
      Assert::That(e.drawItems().empty(), IsTrue());
      Assert::That(e.componentCount(), Equals(static_cast<std::size_t>(0)));
    };

    It(reports_what_it_is_made_of) {
      auto e = Player();
      Assert::That(e->componentCount(), Equals(static_cast<std::size_t>(3)));
      Assert::That(std::string(e->component(0).name()), Equals("INPUT"));
      Assert::That(std::string(e->component(1).name()), Equals("PHYSICS"));
      Assert::That(std::string(e->component(2).name()), Equals("APPEARANCE"));
    };
  };

  // Components act THROUGH the entity. Each one only needs to know the transform
  // and the input -- which is exactly what makes them composable.
  Describe(ComponentsActThroughTheEntity) {
    It(turns_intent_into_velocity) {
      auto e = Player();
      e->update(InputState{false, true, false});    // right
      Assert::That(e->transform().vx, Equals(8.f));
      e->update(InputState{true, false, false});    // left
      Assert::That(e->transform().vx, Equals(-8.f));
    };

    // Both directions at once is not a special case in the code -- the subtraction
    // simply cancels -- but it is pinned so that stays true rather than becoming
    // whichever branch is checked first.
    It(cancels_opposite_intent) {
      auto e = Player();
      e->update(InputState{true, true, false});
      Assert::That(e->transform().vx, Equals(0.f));
    };

    It(integrates_velocity_into_position) {
      auto e = Projectile();
      e->transform().vx = 5.f;
      e->update(InputState{});
      Assert::That(e->transform().x, Equals(5.f));      // moved on x
      Assert::That(e->transform().y < 20.f, IsTrue());  // and fell
    };

    It(lands_and_stops_falling) {
      auto e = Projectile();
      for (int i = 0; i < 40; ++i)
        e->update(InputState{});
      Assert::That(e->transform().y, Equals(0.f));
      Assert::That(e->transform().vy, Equals(0.f));
    };

    It(describes_a_draw_item_from_the_transform) {
      auto e = Prop();
      e->transform().x = 7.f;
      e->transform().y = 3.f;
      e->update(InputState{});
      Assert::That(e->drawItems().size(), Equals(static_cast<std::size_t>(1)));
      Assert::That(e->drawItems()[0].x, Equals(7.f));
      Assert::That(e->drawItems()[0].y, Equals(3.f));
    };

    // ⚠️ THIS CASE WAS WRONG FIRST, AND IT IS THE PATTERN'S OWN SUBJECT THAT MADE
    // IT WRONG: after a full `update()` you observe the SUM of the components, so
    // the physics component has already applied gravity by the time you look. The
    // jump WAS refused, and `vy` is -1.5 -- the gravity -- not 0. Asserting the
    // value the input component would have left alone asserts a frame that does
    // not exist. The refusal is "not a jump", which is what this says now.
    It(jumps_only_from_the_ground) {
      auto e = Player();
      e->transform().y = 5.f;                       // airborne
      e->update(InputState{false, false, true});
      Assert::That(e->transform().vy <= 0.f, IsTrue());   // refused; gravity applied

      e->transform().y = 0.f;                       // grounded
      e->update(InputState{false, false, true});
      Assert::That(e->transform().vy > 0.f, IsTrue());    // accepted, 14 less gravity
    };
  };

  // ⚠️ ORDER IS MEANING, and this is the case that says so: appearance reads the
  // transform, so an appearance placed BEFORE physics describes where the entity
  // was, not where it is. Nothing in the code prevents that -- it is a wiring
  // decision, and this pins what it costs.
  Describe(OrderIsMeaning) {
    It(appearance_before_physics_describes_the_old_position) {
      Entity e{"STALE"};
      e.add(std::make_unique<AppearanceComponent>());
      e.add(std::make_unique<PhysicsComponent>());
      e.transform().vx = 3.f;
      e.update(InputState{});
      Assert::That(e.drawItems()[0].x, Equals(0.f));      // drawn where it was
      Assert::That(e.transform().x, Equals(3.f));         // and then it moved
    };

    It(physics_before_appearance_describes_the_new_one) {
      Entity e{"FRESH"};
      e.add(std::make_unique<PhysicsComponent>());
      e.add(std::make_unique<AppearanceComponent>());
      e.transform().vx = 3.f;
      e.update(InputState{});
      Assert::That(e.drawItems()[0].x, Equals(3.f));      // drawn where it is
    };
  };

  // ⚠️ THE PATTERN'S ACTUAL CLAIM, TESTED AS A CLAIM: three kinds of thing, ONE
  // class, no subclass anywhere -- and they behave differently because they are
  // made differently. If this ever needs a `Player` class, the pattern has been
  // abandoned rather than used.
  Describe(TheSameClassIsEveryKindOfThing) {
    It(a_player_moves_when_it_is_told_to) {
      auto e = Player();
      e->update(InputState{false, true, false});
      Assert::That(e->transform().x > 0.f, IsTrue());
    };

    It(a_projectile_moves_without_being_told_to) {
      auto e = Projectile();
      e->transform().vx = 4.f;                 // set once, by whoever fired it
      e->update(InputState{});                 // no input at all
      Assert::That(e->transform().x, Equals(4.f));
    };

    It(a_prop_never_moves) {
      auto e = Prop();
      e->update(InputState{false, true, true});   // even when told to
      Assert::That(e->transform().x, Equals(0.f));
      Assert::That(e->transform().y, Equals(0.f));
    };

    It(and_all_three_are_the_same_type) {
      // Nothing here is a subclass: the three differ by what they HOLD.
      auto a = Player();
      auto b = Projectile();
      auto c = Prop();
      Assert::That(a->componentCount(), Equals(static_cast<std::size_t>(3)));
      Assert::That(b->componentCount(), Equals(static_cast<std::size_t>(2)));
      Assert::That(c->componentCount(), Equals(static_cast<std::size_t>(1)));
    };
  };

  Describe(DrawItemsAreRebuiltEachFrame) {
    It(does_not_accumulate_them_across_updates) {
      auto e = Prop();
      for (int i = 0; i < 5; ++i)
        e->update(InputState{});
      Assert::That(e->drawItems().size(), Equals(static_cast<std::size_t>(1)));
    };
  };
};
