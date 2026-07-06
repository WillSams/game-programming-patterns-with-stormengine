#include "playState.h"

#include <algorithm>
#include <cmath>

#include "../ui/pixelText.h"

const std::string PlayState::s_playID = "PLAY";

PlayState::PlayState(SDL_Renderer *renderer, int windowWidth, int windowHeight,
                     bool isDebugging, AssetStore_Ptr assetStore, bool &isRunning)
    : renderer_{renderer}, windowWidth_{windowWidth}, windowHeight_{windowHeight},
      isDebugging_{isDebugging}, assetStore_{std::move(assetStore)},
      isRunning_{isRunning}
{
    Reset();
    logger_.Log("PlayState constructor called");
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

void PlayState::Reset() {
    curX_ = prevX_ = windowWidth_ * 0.5f;
    curY_ = prevY_ = windowHeight_ * 0.5f;
    velX_ = 260.f;   // px per second
    velY_ = 180.f;
}

void PlayState::SetRate(float hz) {
    updateHz_ = std::max(1.f, std::min(hz, 120.f));
    loop_ = FixedTimestep(1.f / updateHz_);
    logger_.Log("Update rate: " + std::to_string(static_cast<int>(updateHz_)) + " Hz");
}

// One fixed step: remember where we were, integrate, bounce off the walls.
void PlayState::FixedUpdate() {
    prevX_ = curX_;
    prevY_ = curY_;

    float dt = loop_.step();
    curX_ += velX_ * dt;
    curY_ += velY_ * dt;

    if (curX_ < radius_)                 { curX_ = radius_;                 velX_ = -velX_; }
    if (curX_ > windowWidth_  - radius_) { curX_ = windowWidth_  - radius_; velX_ = -velX_; }
    if (curY_ < radius_)                 { curY_ = radius_;                 velY_ = -velY_; }
    if (curY_ > windowHeight_ - radius_) { curY_ = windowHeight_ - radius_; velY_ = -velY_; }
}

void PlayState::processInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) { isRunning_ = false; return; }
        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE: isRunning_ = false; return;
            case SDLK_SPACE:  interpolate_ = !interpolate_; break;
            case SDLK_UP:     SetRate(updateHz_ + 5.f);     break;
            case SDLK_DOWN:   SetRate(updateHz_ - 5.f);     break;
            case SDLK_r:      Reset();                      break;
            default: break;
            }
        }
    }
}

void PlayState::update() {
    int timeToWait = MILLISECS_PER_FRAME - (SDL_GetTicks() - millisecondsPreviousFrame_);
    if (timeToWait > 0 && timeToWait <= MILLISECS_PER_FRAME)
        SDL_Delay(timeToWait);

    float frameTime = (SDL_GetTicks() - millisecondsPreviousFrame_) / 1000.f;
    millisecondsPreviousFrame_ = SDL_GetTicks();

    // Run as many fixed logic steps as real time has accrued; keep the alpha for
    // this frame's interpolated render.
    StepPlan plan = loop_.advance(frameTime);
    for (int i = 0; i < plan.updates; ++i)
        FixedUpdate();
    alpha_ = plan.alpha;
}

void PlayState::render() {
    SDL_SetRenderDrawColor(renderer_, 16, 18, 24, 255);
    SDL_RenderClear(renderer_);

    // Interpolate between the last two fixed positions for smooth motion; with
    // interpolation off you see the ball jump one whole step at a time.
    float a  = interpolate_ ? alpha_ : 1.f;
    int   cx = static_cast<int>(Lerp(prevX_, curX_, a));
    int   cy = static_cast<int>(Lerp(prevY_, curY_, a));

    SDL_SetRenderDrawColor(renderer_, 120, 200, 255, 255);
    int r = static_cast<int>(radius_);
    for (int dy = -r; dy <= r; ++dy) {
        int span = static_cast<int>(std::sqrt(static_cast<float>(r * r - dy * dy)));
        SDL_RenderDrawLine(renderer_, cx - span, cy + dy, cx + span, cy + dy);
    }

    // Overlay: the fixed update rate and whether interpolation is on.
    SDL_SetRenderDrawColor(renderer_, 230, 230, 235, 255);
    DrawPixelText(renderer_, "RATE " + std::to_string(static_cast<int>(updateHz_)) + " HZ",
                  16, 16, 4);
    DrawPixelText(renderer_, interpolate_ ? "INTERP ON" : "INTERP OFF", 16, 44, 4);

    SDL_RenderPresent(renderer_);
}
