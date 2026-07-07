#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "entity.h"

namespace um {

// The Update Method in action: one loop advances every entity by dt, and each
// runs its own per-frame behavior. Adding a new kind of entity needs no change
// here — it just implements update().
class World {
public:
    void add(std::unique_ptr<Entity> entity) {
        entities_.push_back(std::move(entity));
    }

    void update(float dt) {
        for (auto &entity : entities_)
            entity->update(dt);
    }

    std::size_t count() const { return entities_.size(); }
    Entity     &at(std::size_t i) { return *entities_[i]; }

    // Iteration for rendering.
    auto begin() { return entities_.begin(); }
    auto end()   { return entities_.end(); }

private:
    std::vector<std::unique_ptr<Entity>> entities_;
};

} // namespace um
