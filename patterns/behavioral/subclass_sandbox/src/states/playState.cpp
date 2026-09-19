#include "playState.h"

#include <algorithm>

#include "../ui/pixelText.h"

const std::string PlayState::s_playID = "PLAY";

PlayState::PlayState(SDL_Renderer *renderer, int windowWidth, int windowHeight,
                     bool isDebugging, AssetStore_Ptr assetStore, bool &isRunning)
    : renderer_{renderer}, windowWidth_{windowWidth}, windowHeight_{windowHeight},
      isDebugging_{isDebugging}, assetStore_{std::move(assetStore)},
      isRunning_{isRunning}
{
    logger_.Log("PlayState constructor called");
    powers_.push_back(std::make_unique<powers::SkyLaunch>());
    powers_.push_back(std::make_unique<powers::GroundDive>());
    powers_.push_back(std::make_unique<powers::SuperJump>());
    powers_.push_back(std::make_unique<powers::Fireball>());
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

// The sandbox is reset first, so the EFFECTS panel shows THIS activation. The
// pattern itself accumulates happily -- `specs/` pins that -- but a screen that
// grew a longer list every keypress would be teaching the wrong thing.
void PlayState::Activate(std::size_t index) {
    if (index >= powers_.size())
        return;
    world_.reset();
    index_ = index;
    powers_[index]->activate(world_);
    shown_ = world_.log();
    status_ = "ACTIVATED";   // the name is already the heading; the long
                             // form ran 48px past the window edge
}

void PlayState::processInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) { isRunning_ = false; return; }
        if (event.type != SDL_KEYDOWN)
            continue;

        switch (event.key.keysym.sym) {
        case SDLK_ESCAPE:
            isRunning_ = false;
            return;
        case SDLK_1: Activate(0); break;
        case SDLK_2: Activate(1); break;
        case SDLK_3: Activate(2); break;
        case SDLK_4: Activate(3); break;
        case SDLK_r:
            world_.reset();
            shown_.clear();
            status_ = "READY";
            break;
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
// Same visual language as the bytecode demo: background, ink, 16px padding, 4x
// font, the hint anchored to the window height. The demos are meant to read as one
// set -- see CLAUDE.md.
namespace {
const SDL_Color kInk    = {230, 230, 235, 255};
const SDL_Color kDim    = {140, 150, 165, 255};
const SDL_Color kRow    = {170, 180, 195, 255};
const SDL_Color kCursor = {255, 200,  80, 255};
} // namespace

void PlayState::DrawPowers(int x, int y, int scale) const {
    SDL_SetRenderDrawColor(renderer_, kInk.r, kInk.g, kInk.b, 255);
    DrawPixelText(renderer_, "POWERS", x, y, scale);
    for (std::size_t i = 0; i < powers_.size(); ++i) {
        const bool on = (i == index_ && !shown_.empty());
        SDL_SetRenderDrawColor(renderer_, on ? kCursor.r : kRow.r,
                               on ? kCursor.g : kRow.g,
                               on ? kCursor.b : kRow.b, 255);
        DrawPixelText(renderer_,
                      (on ? "> " : "  ") + std::to_string(i + 1) + " " +
                          powers_[i]->name(),
                      x, y + 24 + static_cast<int>(i) * 20, 3);
    }
}

void PlayState::DrawEffects(int x, int y, int scale) const {
    SDL_SetRenderDrawColor(renderer_, kInk.r, kInk.g, kInk.b, 255);
    DrawPixelText(renderer_, "EFFECTS", x, y, scale);
    SDL_SetRenderDrawColor(renderer_, kRow.r, kRow.g, kRow.b, 255);
    if (shown_.empty()) {
        DrawPixelText(renderer_, "NOTHING YET", x, y + 24, 3);
        return;
    }
    for (std::size_t i = 0; i < shown_.size(); ++i)
        DrawPixelText(renderer_, powers::DescribeEffect(shown_[i]), x,
                      y + 24 + static_cast<int>(i) * 18, 3);
}

void PlayState::DrawWorld(int x, int y, int scale) const {
    SDL_SetRenderDrawColor(renderer_, kInk.r, kInk.g, kInk.b, 255);
    DrawPixelText(renderer_, "WORLD", x, y, scale);
    SDL_SetRenderDrawColor(renderer_, kRow.r, kRow.g, kRow.b, 255);
    // 12px of daylight between these two, not 0: at scale 4 a line is 20px tall,
    // so a 20px pitch butts them together and they read as one smear.
    DrawPixelText(renderer_, "HP " + std::to_string(world_.health()), x, y + 32, scale);
    DrawPixelText(renderer_,
                  "X " + std::to_string(world_.x()) + " Y " + std::to_string(world_.y()),
                  x, y + 64, scale);

    // The world as a box with the power's effect on it: y is up on screen because
    // "jump" means UP, while the sandbox counts +y downward. The mapping is stated
    // here rather than hidden, because it is the one place the two disagree.
    const int boxW = std::min(240, windowWidth_ / 3);
    const int boxH = 160;
    const int top  = y + 84;
    SDL_SetRenderDrawColor(renderer_, 40, 46, 58, 255);
    SDL_Rect box = {x, top, boxW, boxH};
    SDL_RenderFillRect(renderer_, &box);
    SDL_SetRenderDrawColor(renderer_, 70, 80, 95, 255);
    SDL_RenderDrawRect(renderer_, &box);

    // Middle line, so up and down are readable at a glance.
    SDL_SetRenderDrawColor(renderer_, 70, 80, 95, 255);
    SDL_RenderDrawLine(renderer_, x, top + boxH / 2, x + boxW, top + boxH / 2);

    const int dotX = x + boxW / 2 + std::clamp(world_.x(), -100, 100) * (boxW / 2) / 100;
    // ⚠️ THE SIGN HERE WAS WRONG, and the comment above said so while the code did
    // the opposite: the sandbox counts +y DOWNWARD, so a jump (y = -80) has to move
    // the dot UP the screen. The first version subtracted, so Super Jump drew its
    // dot BELOW the midline -- a visual that contradicted its own caption, and
    // exactly the kind of thing only looking at it catches.
    const int dotY = top + boxH / 2 + std::clamp(world_.y(), -100, 100) * (boxH / 2) / 100;
    SDL_Rect dot = {dotX - 4, dotY - 4, 8, 8};
    SDL_SetRenderDrawColor(renderer_, 120, 200, 255, 255);
    SDL_RenderFillRect(renderer_, &dot);
}

void PlayState::DrawHint(int x, int y, int scale) const {
    SDL_SetRenderDrawColor(renderer_, kDim.r, kDim.g, kDim.b, 255);
    DrawPixelText(renderer_, "1-4 POWER  R RESET  ESC QUIT", x, y, scale);
}

void PlayState::render() {
    SDL_SetRenderDrawColor(renderer_, 16, 18, 24, 255);
    SDL_RenderClear(renderer_);

    const int pad = 16;
    const int scale = 4;
    const int colR = windowWidth_ / 2 + pad;

    SDL_SetRenderDrawColor(renderer_, kInk.r, kInk.g, kInk.b, 255);
    DrawPixelText(renderer_, "SUBCLASS SANDBOX", pad, pad, scale);

    SDL_SetRenderDrawColor(renderer_, kDim.r, kDim.g, kDim.b, 255);
    DrawPixelText(renderer_,
                  shown_.empty() ? "PRESS 1-4 TO ACTIVATE A POWER"
                                 : powers_[index_]->name(),
                  pad, pad + 28, scale);

    DrawPowers(pad, 96, scale);
    DrawEffects(pad, 348, scale);

    DrawWorld(colR, 96, scale);

    SDL_SetRenderDrawColor(renderer_, kInk.r, kInk.g, kInk.b, 255);
    DrawPixelText(renderer_, "STATUS " + status_, colR, 348, scale);

    DrawHint(pad, windowHeight_ - 32, 3);

    SDL_RenderPresent(renderer_);
}
