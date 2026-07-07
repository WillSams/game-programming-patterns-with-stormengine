#include <igloo/igloo_alt.h>
#include <memory>

#include "../src/entities/skeleton.h"
#include "../src/entities/statue.h"
#include "../src/entities/world.h"

using namespace igloo;
using namespace um;

Describe(SkeletonSpec) {

    It(should_advance_in_its_heading_each_update) {
        Skeleton s(50.f, 0.f, 0.f, 100.f, 10.f); // speed 10 px/s
        s.update(1.0f);
        Assert::That(s.position().x, EqualsWithDelta(60.f, 1e-4));
        Assert::That(s.direction(), Equals(1.f));
    };

    It(should_reverse_and_clamp_at_the_far_end) {
        Skeleton s(95.f, 0.f, 0.f, 100.f, 10.f);
        s.update(1.0f);                 // would reach 105, clamps to 100 and flips
        Assert::That(s.position().x, EqualsWithDelta(100.f, 1e-4));
        Assert::That(s.direction(), Equals(-1.f));
    };

    It(should_reverse_and_clamp_at_the_near_end) {
        Skeleton s(10.f, 0.f, 0.f, 100.f, 100.f); // fast: crosses a wall each step
        s.update(1.0f);                 // 10 -> 110, clamps to 100, dir -1
        Assert::That(s.direction(), Equals(-1.f));
        s.update(1.0f);                 // 100 -> 0, clamps to 0, dir +1
        Assert::That(s.position().x, EqualsWithDelta(0.f, 1e-4));
        Assert::That(s.direction(), Equals(1.f));
    };
};

Describe(StatueSpec) {

    It(should_not_fire_before_its_period_elapses) {
        Statue statue(0.f, 0.f, 1.0f);
        statue.update(0.6f);
        Assert::That(statue.fired(), Equals(0));
    };

    It(should_fire_once_when_the_period_elapses) {
        Statue statue(0.f, 0.f, 1.0f);
        statue.update(0.6f);
        statue.update(0.6f); // 1.2s total -> one shot
        Assert::That(statue.fired(), Equals(1));
    };

    It(should_fire_multiple_times_across_a_long_update) {
        Statue statue(0.f, 0.f, 1.0f);
        statue.update(3.5f); // three full periods
        Assert::That(statue.fired(), Equals(3));
    };

    It(should_glow_right_after_firing) {
        Statue statue(0.f, 0.f, 0.02f);
        statue.update(0.02f); // fires this frame; glow barely decays over 20ms
        Assert::That(statue.glow() > 0.f, Equals(true));
    };
};

Describe(WorldSpec) {

    It(should_advance_every_entity_on_one_update) {
        World world;
        world.add(std::make_unique<Skeleton>(0.f, 0.f, 0.f, 100.f, 10.f));
        world.add(std::make_unique<Statue>(50.f, 50.f, 1.0f));

        world.update(1.0f);

        // Skeleton moved, statue fired — each ran its own behavior from one loop.
        Assert::That(world.at(0).position().x, EqualsWithDelta(10.f, 1e-4));
        Assert::That(static_cast<Statue &>(world.at(1)).fired(), Equals(1));
    };

    It(should_report_how_many_entities_it_holds) {
        World world;
        world.add(std::make_unique<Skeleton>(0.f, 0.f, 0.f, 10.f, 1.f));
        world.add(std::make_unique<Skeleton>(0.f, 0.f, 0.f, 10.f, 1.f));
        Assert::That(world.count(), Equals(2u));
    };
};
