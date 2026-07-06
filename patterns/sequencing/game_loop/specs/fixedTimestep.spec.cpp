#include <igloo/igloo_alt.h>

#include "../src/loop/fixedTimestep.h"

using namespace igloo;

// A 60 Hz step is ~0.016667s; use a round 0.1s step so the arithmetic is exact.
Describe(FixedTimestepSpec) {

    It(should_run_one_update_when_a_full_step_elapses) {
        FixedTimestep loop(0.1f);
        StepPlan plan = loop.advance(0.1f);
        Assert::That(plan.updates, Equals(1));
        Assert::That(plan.alpha, EqualsWithDelta(0.f, 1e-4));
    };

    It(should_run_no_updates_for_a_frame_shorter_than_a_step) {
        FixedTimestep loop(0.1f);
        StepPlan plan = loop.advance(0.04f);
        Assert::That(plan.updates, Equals(0));
    };

    It(should_carry_the_remainder_as_alpha) {
        FixedTimestep loop(0.1f);
        StepPlan plan = loop.advance(0.15f); // one step, 0.05 left over
        Assert::That(plan.updates, Equals(1));
        Assert::That(plan.alpha, EqualsWithDelta(0.5f, 1e-4)); // 0.05 / 0.1
    };

    It(should_run_multiple_updates_for_a_long_frame) {
        FixedTimestep loop(0.1f);
        StepPlan plan = loop.advance(0.30f);
        Assert::That(plan.updates, Equals(3));
        Assert::That(plan.alpha, EqualsWithDelta(0.f, 1e-4));
    };

    It(should_accumulate_leftover_time_across_frames) {
        FixedTimestep loop(0.1f);
        Assert::That(loop.advance(0.06f).updates, Equals(0)); // banked 0.06
        StepPlan plan = loop.advance(0.06f);                  // 0.12 total -> one step
        Assert::That(plan.updates, Equals(1));
        Assert::That(plan.alpha, EqualsWithDelta(0.2f, 1e-4)); // 0.02 / 0.1
    };

    It(should_cap_updates_and_drop_the_backlog_to_avoid_a_spiral) {
        FixedTimestep loop(0.1f, 3);   // cap at 3 updates/frame
        StepPlan plan = loop.advance(1.0f); // would be 10 steps uncapped
        Assert::That(plan.updates, Equals(3));
        Assert::That(plan.alpha, EqualsWithDelta(0.f, 1e-4)); // backlog discarded
    };
};

Describe(LerpSpec) {
    It(should_return_the_previous_value_at_alpha_zero) {
        Assert::That(Lerp(10.f, 20.f, 0.f), Equals(10.f));
    };
    It(should_return_the_current_value_at_alpha_one) {
        Assert::That(Lerp(10.f, 20.f, 1.f), Equals(20.f));
    };
    It(should_return_the_midpoint_at_alpha_half) {
        Assert::That(Lerp(10.f, 20.f, 0.5f), Equals(15.f));
    };
};
