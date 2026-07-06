#pragma once

#include <algorithm>

// ── Game Loop pattern ────────────────────────────────────────────────────────
// (Game Programming Patterns, Robert Nystrom — Sequencing Patterns)
//
// The interesting, testable half of a game loop is decoupling *update* rate from
// *render* rate. FixedTimestep accumulates real elapsed time and hands back a
// plan: how many fixed-size logic updates to run this frame, plus an `alpha` in
// [0,1) — the leftover fraction of a step — used to interpolate rendering so
// motion stays smooth no matter how fast frames are drawn.
//
// A hard cap on updates per frame prevents the "spiral of death": if a single
// frame takes too long, running one update per elapsed step would queue even
// more work, and the sim would never catch up. Past the cap we drop the backlog.

struct StepPlan {
    int   updates = 0;   // number of fixed logic updates to run this frame
    float alpha   = 0.f; // render interpolation factor in [0, 1)
};

class FixedTimestep {
public:
    // step: seconds per logic update (e.g. 1/60). maxUpdates: spiral-of-death cap.
    explicit FixedTimestep(float step, int maxUpdates = 5)
        : step_(step), maxUpdates_(maxUpdates) {}

    // Fold this frame's elapsed time into the accumulator and return the plan.
    StepPlan advance(float frameTime) {
        accumulator_ += frameTime;

        int updates = 0;
        while (accumulator_ >= step_ && updates < maxUpdates_) {
            accumulator_ -= step_;
            ++updates;
        }

        // Hit the cap with time still banked: discard the backlog rather than
        // let it snowball frame after frame.
        if (updates == maxUpdates_ && accumulator_ >= step_)
            accumulator_ = 0.f;

        float alpha = std::min(accumulator_ / step_, 1.f);
        return { updates, alpha };
    }

    float step() const { return step_; }

private:
    float step_;
    int   maxUpdates_;
    float accumulator_ = 0.f;
};

// Linear interpolation between a previous and current value by alpha — how the
// leftover accumulator turns two fixed states into a smooth rendered position.
inline float Lerp(float previous, float current, float alpha) {
    return previous + (current - previous) * alpha;
}
