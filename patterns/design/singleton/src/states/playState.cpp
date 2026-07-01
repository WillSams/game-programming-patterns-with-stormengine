#include "playState.h"

#include "../Settings.h"

const std::string PlayState::s_playID = "PLAY";

PlayState::PlayState(SDL_Renderer *renderer, int windowWidth, int windowHeight,
                     bool isDebugging, AssetStore_Ptr assetStore, bool &isRunning)
    : renderer_{renderer}, windowWidth_{windowWidth}, windowHeight_{windowHeight},
      isDebugging_{isDebugging}, assetStore_{std::move(assetStore)},
      isRunning_{isRunning}
{
    logger_.Log("Singleton PlayState constructor called");
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
            // Mutate the one global instance from here.
            Settings &s = Settings::instance();
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE: isRunning_ = false; return;
            case SDLK_UP:    s.setVolume(s.volume() + 5); break;
            case SDLK_DOWN:  s.setVolume(s.volume() - 5); break;
            case SDLK_d:     s.cycleDifficulty();         break;
            default: break;
            }
        }
    }
}

void PlayState::update() {
    int timeToWait = MILLISECS_PER_FRAME - (SDL_GetTicks() - millisecondsPreviousFrame_);
    if (timeToWait > 0 && timeToWait <= MILLISECS_PER_FRAME)
        SDL_Delay(timeToWait);
    millisecondsPreviousFrame_ = SDL_GetTicks();
}

void PlayState::render() {
    SDL_SetRenderDrawColor(renderer_, 24, 28, 38, 255);
    SDL_RenderClear(renderer_);

    // Read the same global instance here — no Settings member in sight.
    const Settings &s = Settings::instance();

    // Volume bar (Up/Down).
    const int barX = 80, barY = 240, barW = windowWidth_ - 160, barH = 40;
    SDL_SetRenderDrawColor(renderer_, 60, 66, 80, 255);
    SDL_Rect track = {barX, barY, barW, barH};
    SDL_RenderFillRect(renderer_, &track);
    SDL_SetRenderDrawColor(renderer_, 120, 200, 255, 255);
    SDL_Rect fill = {barX, barY, barW * s.volume() / 100, barH};
    SDL_RenderFillRect(renderer_, &fill);

    // Difficulty pips (D) — Easy=1, Normal=2, Hard=3 lit.
    int lit = static_cast<int>(s.difficulty()) + 1;
    for (int i = 0; i < 3; i++) {
        if (i < lit) SDL_SetRenderDrawColor(renderer_, 255, 200, 80, 255);
        else         SDL_SetRenderDrawColor(renderer_, 60, 66, 80, 255);
        SDL_Rect pip = {barX + i * 60, barY + 90, 44, 44};
        SDL_RenderFillRect(renderer_, &pip);
    }

    SDL_RenderPresent(renderer_);
}
