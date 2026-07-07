#pragma once

#include "entity.h"

namespace um {

// The book's patrolling guard: walks horizontally between [minX, maxX], reversing
// at each end. Its own update() carries the current heading between frames — the
// essence of the pattern.
class Skeleton : public Entity {
public:
    Skeleton(float x, float y, float minX, float maxX, float speed)
        : minX_(minX), maxX_(maxX), speed_(speed) {
        pos_ = { x, y };
    }

    void update(float dt) override {
        pos_.x += dir_ * speed_ * dt;
        if (pos_.x <= minX_) { pos_.x = minX_; dir_ =  1.f; }
        if (pos_.x >= maxX_) { pos_.x = maxX_; dir_ = -1.f; }
    }

    float direction() const { return dir_; } // +1 moving right, -1 moving left

private:
    float minX_, maxX_, speed_;
    float dir_ = 1.f;
};

} // namespace um
