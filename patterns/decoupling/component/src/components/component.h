#pragma once

#include <memory>
#include <string>
#include <vector>

// ── Component pattern: an entity is a CONTAINER, not a hierarchy ─────────────
//
// The chapter's problem: one class per kind of thing does not scale. A player
// needs input and physics and drawing; a projectile needs physics and drawing but
// no input; a decoration needs drawing alone. Inheritance gives you a base class
// and a subclass per combination, or one bloated base with flags in it.
//
// The pattern replaces that with COMPOSITION: one `Entity`, and behaviour supplied
// by the components it holds. What an entity IS becomes what it is MADE OF.
//
// ⚠️ PURE, AND THE DEMO'S DRAWING IS NOT IN HERE. A component's job in this
// implementation is to change the transform and to APPEND A DRAW ITEM -- a plain
// float pair and a size -- which the SDL layer later draws. That keeps the pattern
// spec-able with no window (CODING.md tenet 9: rules in the pure headers, branches
// in the glue), and it is also what the chapter's `GraphicsComponent` is really
// doing: describing, not drawing.
namespace components {

// What the outside world says this frame. Plain data, so a spec can write one and
// a demo can build one from a keypress -- no SDL event in the pure layer.
struct InputState {
    bool left = false;
    bool right = false;
    bool jump = false;
};

// Where an entity is, and how fast. Shared by every component that acts on it,
// which is the whole reason components talk through the entity rather than to each
// other.
struct Transform {
    float x = 0.f;
    float y = 0.f;
    float vx = 0.f;
    float vy = 0.f;
};

// One thing to draw. Deliberately not an SDL_Rect: the pure layer describes, the
// glue renders (tenet 9).
struct DrawItem {
    float x = 0.f;
    float y = 0.f;
    float size = 0.f;
};

class Entity;

// ⚠️ THE INTERFACE IS THE SEAM, AND IT IS SMALL ON PURPOSE. A component may change
// the transform, read the input, and append draw items. It cannot reach the world,
// the renderer or another entity -- an entity is handed to it, nothing else. That
// is the chapter's "components should not know about each other", as a fact rather
// than advice.
class Component {
public:
    virtual ~Component() = default;
    virtual void update(Entity &entity, const InputState &input) = 0;
    virtual const char *name() const = 0;
};

class Entity {
public:
    Entity(std::string name) : name_{std::move(name)} {}

    // Components are added in the order they should run. ⚠️ ORDER IS MEANING:
    // physics before appearance, or a sprite is drawn a frame behind its position.
    // That is stated here rather than left to be discovered.
    void add(std::unique_ptr<Component> component) {
        components_.push_back(std::move(component));
    }

    void update(const InputState &input) {
        draw_.clear();   // rebuilt each frame, so a removed component cannot leave
                         // a stale item behind
        for (auto &c : components_)
            c->update(*this, input);
    }

    Transform       &transform() { return transform_; }
    const Transform &transform() const { return transform_; }
    const std::vector<DrawItem> &drawItems() const { return draw_; }
    void draw(const DrawItem &item) { draw_.push_back(item); }

    const std::string &name() const { return name_; }
    std::size_t componentCount() const { return components_.size(); }
    const Component &component(std::size_t i) const { return *components_[i]; }

private:
    std::string                             name_;
    Transform                               transform_;
    std::vector<std::unique_ptr<Component>> components_;
    std::vector<DrawItem>                   draw_;
};

} // namespace components
