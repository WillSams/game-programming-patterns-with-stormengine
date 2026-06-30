#include "playState.h"

#include <cmath>
#include <string>

const std::string PlayState::s_playID = "PLAY";

PlayState::PlayState(SDL_Renderer *renderer, int windowWidth, int windowHeight,
                     bool isDebugging, AssetStore_Ptr assetStore, bool &isRunning)
    : renderer_{renderer}, windowWidth_{windowWidth}, windowHeight_{windowHeight},
      isDebugging_{isDebugging}, assetStore_{std::move(assetStore)},
      isRunning_{isRunning}
{
    logger_.Log("State PlayState constructor called");
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
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE: isRunning_ = false; return;
            case SDLK_SPACE: character_.handleInput(Input::Jump);  break;
            case SDLK_DOWN:  character_.handleInput(Input::Duck);  break;
            case SDLK_UP:    character_.handleInput(Input::Stand); break;
            default: break;
            }
        }
        if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_DOWN) {
            character_.handleInput(Input::Stand); // release duck -> stand
        }
    }
}

void PlayState::update() {
    int timeToWait = MILLISECS_PER_FRAME - (SDL_GetTicks() - millisecondsPreviousFrame_);
    if (timeToWait > 0 && timeToWait <= MILLISECS_PER_FRAME)
        SDL_Delay(timeToWait);
    float dt = (SDL_GetTicks() - millisecondsPreviousFrame_) / 1000.f;
    dt = std::min(dt, 0.05f);
    millisecondsPreviousFrame_ = SDL_GetTicks();

    character_.update(dt);
}

void PlayState::render() {
    SDL_SetRenderDrawColor(renderer_, 24, 28, 38, 255);
    SDL_RenderClear(renderer_);

    const int groundY = static_cast<int>(windowHeight_ * 0.72f);

    // Ground line.
    SDL_SetRenderDrawColor(renderer_, 70, 78, 92, 255);
    SDL_Rect ground = {0, groundY, windowWidth_, 4};
    SDL_RenderFillRect(renderer_, &ground);

    // Shape + colour come entirely from the current state.
    std::string st = character_.stateName();
    int w = 44, h = 64, lift = 0;
    SDL_Color color = {120, 200, 255, 255}; // standing (blue)

    if (st == "Jumping") {
        color = {120, 220, 140, 255}; // green
        // A smooth arc over the jump's duration.
        float t   = character_.airTime() / Character::JUMP_DURATION;
        lift = static_cast<int>(std::sin(t * 3.14159265f) * 150.f);
    } else if (st == "Ducking") {
        color = {245, 175, 80, 255};  // orange
        w = 64; h = 32;               // short & wide
    }

    SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, 255);
    SDL_Rect body = {windowWidth_ / 2 - w / 2, groundY - h - lift, w, h};
    SDL_RenderFillRect(renderer_, &body);

    SDL_RenderPresent(renderer_);
}
