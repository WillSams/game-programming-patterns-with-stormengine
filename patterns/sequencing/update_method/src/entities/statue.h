#pragma once

#include <algorithm>

#include "entity.h"

namespace um {

// The book's enchanted statue: stationary, shoots lightning every `period`
// seconds, tracking its own timer across frames and counting total shots. `glow`
// decays after each shot purely for rendering (flares on fire, fades out).
class Statue : public Entity {
public:
    Statue(float x, float y, float period) : period_(period) {
        pos_ = { x, y };
    }

    void update(float dt) override {
        timer_ += dt;
        while (timer_ >= period_) {
            timer_ -= period_;
            ++fired_;
            glow_ = 1.f;
        }
        glow_ = std::max(0.f, glow_ - dt * 3.f);
    }

    int   fired() const { return fired_; } // total shots so far
    float glow()  const { return glow_; }  // 1 just after firing, fades to 0

private:
    float period_;
    float timer_ = 0.f;
    int   fired_ = 0;
    float glow_  = 0.f;
};

} // namespace um
