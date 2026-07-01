#include "playState.h"

const std::string PlayState::s_playID = "PLAY";

PlayState::PlayState(SDL_Renderer *renderer, int windowWidth, int windowHeight,
                     bool isDebugging, AssetStore_Ptr assetStore, bool &isRunning)
    : renderer_{renderer}, windowWidth_{windowWidth}, windowHeight_{windowHeight},
      isDebugging_{isDebugging}, assetStore_{std::move(assetStore)},
      isRunning_{isRunning}
{
    // Register both observers — the subject just gained two listeners and
    // remains unaware of what they do.
    subject_.addObserver(&hud_);
    subject_.addObserver(&milestone_);
    logger_.Log("Observer PlayState constructor called");
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

void PlayState::processInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) { isRunning_ = false; return; }
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) { isRunning_ = false; return; }
            if (event.key.keysym.sym == SDLK_SPACE)  { subject_.addPoints(10); }
        }
    }
}

void PlayState::update() {
    int timeToWait = MILLISECS_PER_FRAME - (SDL_GetTicks() - millisecondsPreviousFrame_);
    if (timeToWait > 0 && timeToWait <= MILLISECS_PER_FRAME)
        SDL_Delay(timeToWait);
    float dt = (SDL_GetTicks() - millisecondsPreviousFrame_) / 1000.f;
    millisecondsPreviousFrame_ = SDL_GetTicks();

    // The milestone observer reacted to the score change; the HUD reacted too.
    // Here we only consume the milestone's one-shot to start a flash.
    if (milestone_.consumeJustReached())
        flashTimer_ = 0.6f;
    if (flashTimer_ > 0.f)
        flashTimer_ -= dt;
}

void PlayState::render() {
    SDL_SetRenderDrawColor(renderer_, 24, 28, 38, 255);
    SDL_RenderClear(renderer_);

    // HUD observer: a score bar that grows 1px per point.
    const int barH = 40;
    const int barY = windowHeight_ / 2 - barH / 2;
    int barW = hud_.displayed();
    if (barW > windowWidth_ - 80) barW = windowWidth_ - 80;

    SDL_SetRenderDrawColor(renderer_, 120, 200, 255, 255);
    SDL_Rect bar = {40, barY, barW, barH};
    SDL_RenderFillRect(renderer_, &bar);

    // Milestone markers every 100 points.
    SDL_SetRenderDrawColor(renderer_, 80, 90, 110, 255);
    for (int x = 40 + 100; x < windowWidth_ - 40; x += 100) {
        SDL_Rect tick = {x, barY - 8, 2, barH + 16};
        SDL_RenderFillRect(renderer_, &tick);
    }

    // Milestone observer: flash a gold overlay when one is crossed.
    if (flashTimer_ > 0.f) {
        Uint8 a = static_cast<Uint8>(180 * (flashTimer_ / 0.6f));
        SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer_, 255, 215, 0, a);
        SDL_Rect full = {0, 0, windowWidth_, windowHeight_};
        SDL_RenderFillRect(renderer_, &full);
    }

    SDL_RenderPresent(renderer_);
}
