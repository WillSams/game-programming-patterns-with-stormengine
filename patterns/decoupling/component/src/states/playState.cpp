#include "playState.h"

#include <algorithm>

#include "pixelText.h"    // shared: include/pixelText.h

const std::string PlayState::s_playID = "PLAY";

PlayState::PlayState(SDL_Renderer *renderer, int windowWidth, int windowHeight,
                     bool isDebugging, AssetStore_Ptr assetStore, bool &isRunning)
    : renderer_{renderer}, windowWidth_{windowWidth}, windowHeight_{windowHeight},
      isDebugging_{isDebugging}, assetStore_{std::move(assetStore)},
      isRunning_{isRunning}
{
    logger_.Log("PlayState constructor called");
    Reset();
}

PlayState::~PlayState() { onExit(); }

bool PlayState::onEnter() {
    m_loadingComplete = true;
    return true;
}

bool PlayState::onExit() {
    assetStore_->ClearAssets();
    m_exiting = true;
    return true;
}

// The three kinds of thing, built from the same three parts. NOTHING here is a
// subclass: the difference between them is entirely what they hold.
void PlayState::Reset() {
    entities_.clear();

    auto player = std::make_unique<components::Entity>("PLAYER");
    player->add(std::make_unique<components::InputComponent>());
    player->add(std::make_unique<components::PhysicsComponent>());
    player->add(std::make_unique<components::AppearanceComponent>(14.f));

    auto shot = std::make_unique<components::Entity>("PROJECTILE");
    shot->transform().x = -30.f;
    shot->transform().y = 24.f;
    shot->transform().vx = 5.f;      // set once, by whoever "fired" it
    shot->add(std::make_unique<components::PhysicsComponent>());
    shot->add(std::make_unique<components::AppearanceComponent>(8.f));

    auto prop = std::make_unique<components::Entity>("PROP");
    prop->transform().x = 26.f;
    prop->transform().y = 6.f;
    prop->add(std::make_unique<components::AppearanceComponent>(18.f));

    entities_.push_back(std::move(player));
    entities_.push_back(std::move(shot));
    entities_.push_back(std::move(prop));

    ticks_ = 0;
    status_ = "READY";
}

void PlayState::Tick(const components::InputState &input, int times) {
    for (int i = 0; i < times; ++i) {
        for (auto &e : entities_)
            e->update(input);
        ++ticks_;
    }
    status_ = "TICK " + std::to_string(ticks_);
}

void PlayState::processInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) { isRunning_ = false; return; }
        if (event.type != SDL_KEYDOWN)
            continue;

        components::InputState in;
        switch (event.key.keysym.sym) {
        case SDLK_ESCAPE:
            isRunning_ = false;
            return;
        case SDLK_a:
        case SDLK_LEFT:
            in.left = true;
            Tick(in);
            break;
        case SDLK_d:
        case SDLK_RIGHT:
            in.right = true;
            Tick(in);
            break;
        case SDLK_SPACE:
        case SDLK_UP:
            in.jump = true;
            Tick(in);
            break;
        case SDLK_t:                     // let the projectile fall
            Tick(components::InputState{}, 10);
            break;
        case SDLK_r:
            Reset();
            break;
        case SDLK_1: index_ = 0; status_ = "INSPECT PLAYER"; break;
        case SDLK_2: index_ = 1; status_ = "INSPECT PROJECTILE"; break;
        case SDLK_3: index_ = 2; status_ = "INSPECT PROP"; break;
        default: break;
        }
    }
}

void PlayState::update() {
    int timeToWait = MILLISECS_PER_FRAME - (SDL_GetTicks() - millisecondsPreviousFrame_);
    if (timeToWait > 0 && timeToWait <= MILLISECS_PER_FRAME)
        SDL_Delay(timeToWait);
    millisecondsPreviousFrame_ = SDL_GetTicks();
}

// ── The screen ───────────────────────────────────────────────────────────────
//
// Same visual language as the other demos: background, ink, 16px padding, 4x
// font, hint anchored to the window height.
namespace {
const SDL_Color kInk    = {230, 230, 235, 255};
const SDL_Color kDim    = {140, 150, 165, 255};
const SDL_Color kRow    = {170, 180, 195, 255};
const SDL_Color kCursor = {255, 200,  80, 255};
} // namespace

// What each entity IS: its name and the parts it holds. This is the pattern on
// screen -- the list of components IS the entity's behaviour.
void PlayState::DrawEntities(int x, int y, int scale) const {
    SDL_SetRenderDrawColor(renderer_, kInk.r, kInk.g, kInk.b, 255);
    DrawPixelText(renderer_, "ENTITIES", x, y, scale);
    for (std::size_t i = 0; i < entities_.size(); ++i) {
        const bool on = (i == index_);
        SDL_SetRenderDrawColor(renderer_, on ? kCursor.r : kRow.r,
                               on ? kCursor.g : kRow.g,
                               on ? kCursor.b : kRow.b, 255);
        std::string parts;
        for (std::size_t c = 0; c < entities_[i]->componentCount(); ++c) {
            if (!parts.empty())
                parts += " ";
            parts += entities_[i]->component(c).name();
        }
        DrawPixelText(renderer_,
                      (on ? "> " : "  ") + entities_[i]->name() + "  " + parts,
                      x, y + 24 + static_cast<int>(i) * 20, 3);
    }
}

void PlayState::DrawInspect(int x, int y, int scale) const {
    const components::Entity &e = *entities_[index_];
    const components::Transform &t = e.transform();
    SDL_SetRenderDrawColor(renderer_, kInk.r, kInk.g, kInk.b, 255);
    DrawPixelText(renderer_, "INSPECT " + e.name(), x, y, scale);
    SDL_SetRenderDrawColor(renderer_, kRow.r, kRow.g, kRow.b, 255);
    DrawPixelText(renderer_,
                  "X " + std::to_string(static_cast<int>(t.x)) +
                      " Y " + std::to_string(static_cast<int>(t.y)),
                  x, y + 32, scale);
    DrawPixelText(renderer_,
                  "VX " + std::to_string(static_cast<int>(t.vx)) +
                      " VY " + std::to_string(static_cast<int>(t.vy)),
                  x, y + 64, scale);
}

// The draw items the appearance components produced, on a ground line. The pure
// layer described these; this is the only place SDL is involved.
void PlayState::DrawWorld(int x, int y, int scale) const {
    const int padW = 16;
    SDL_SetRenderDrawColor(renderer_, kInk.r, kInk.g, kInk.b, 255);
    DrawPixelText(renderer_, "WORLD", x, y, scale);

    // Derived from the room LEFT, not a literal, so the panel cannot run
    // past the window at any column position (see the colR note).
    const int boxW = std::min(280, windowWidth_ - x - padW);
    const int boxH = 200;
    const int top  = y + 32;
    SDL_SetRenderDrawColor(renderer_, 40, 46, 58, 255);
    SDL_Rect box = {x, top, boxW, boxH};
    SDL_RenderFillRect(renderer_, &box);

    // Ground: the physics floor is y = 0, so it sits at the bottom of the box.
    const int groundY = top + boxH - 24;
    SDL_SetRenderDrawColor(renderer_, 70, 80, 95, 255);
    SDL_RenderDrawLine(renderer_, x, groundY, x + boxW, groundY);

    // x in [-60, 60] across the box; y upward from the ground, 1 unit = 4px.
    for (const auto &e : entities_) {
        for (const components::DrawItem &d : e->drawItems()) {
            const int px = x + boxW / 2 + static_cast<int>(d.x * (boxW / 2) / 60.f);
            const int py = groundY - static_cast<int>(d.y * 4.f);
            const int s  = std::max(4, static_cast<int>(d.size));
            SDL_Rect r = {px - s / 2, py - s, s, s};
            // `&e` is the unique_ptr, not the entity -- compare the pointees.
            const bool on = (e.get() == entities_[index_].get());
            SDL_SetRenderDrawColor(renderer_, on ? 255 : 120, on ? 200 : 200,
                                   on ? 80 : 255, 255);
            SDL_RenderFillRect(renderer_, &r);
        }
    }
}

void PlayState::DrawHint(int x, int y, int scale) const {
    SDL_SetRenderDrawColor(renderer_, kDim.r, kDim.g, kDim.b, 255);
    DrawPixelText(renderer_, "A D MOVE  SPACE JUMP  T TEN TICKS  R RESET  1-3 INSPECT",
                  x, y, scale);
}

void PlayState::render() {
    SDL_SetRenderDrawColor(renderer_, 16, 18, 24, 255);
    SDL_RenderClear(renderer_);

    const int pad = 16;
    const int scale = 4;
    // ⚠️ NOT windowWidth_/2. The longest row here is "PLAYER  INPUT PHYSICS
    // APPEARANCE" -- 33 characters at scale 3 is 396px from x=416 -- so a
    // half-window column at 416 was 8px INSIDE it, and the WORLD panel (drawn
    // after the list) painted over the last glyph's lower half. Measured: the text
    // occupied rows 124-134 and the panel covered rows 128+ past x=416. Three
    // fifths leaves 56px of clearance and still fits the panel.
    const int colR = windowWidth_ * 3 / 5;

    SDL_SetRenderDrawColor(renderer_, kInk.r, kInk.g, kInk.b, 255);
    DrawPixelText(renderer_, "COMPONENT", pad, pad, scale);

    SDL_SetRenderDrawColor(renderer_, kDim.r, kDim.g, kDim.b, 255);
    DrawPixelText(renderer_, "ONE CLASS, MADE OF PARTS", pad, pad + 28, scale);

    DrawEntities(pad, 96, scale);
    DrawInspect(pad, 240, scale);

    DrawWorld(colR, 96, scale);

    SDL_SetRenderDrawColor(renderer_, kInk.r, kInk.g, kInk.b, 255);
    DrawPixelText(renderer_, "STATUS " + status_, colR, 348, scale);

    DrawHint(pad, windowHeight_ - 32, 3);

    SDL_RenderPresent(renderer_);
}
