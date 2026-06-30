#include "playState.h"

const std::string PlayState::s_playID = "PLAY";

PlayState::PlayState(SDL_Renderer *renderer, int windowWidth, int windowHeight,
                     bool isDebugging, AssetStore_Ptr assetStore, bool &isRunning)
    : renderer_{renderer}, windowWidth_{windowWidth}, windowHeight_{windowHeight},
      isDebugging_{isDebugging}, assetStore_{std::move(assetStore)},
      isRunning_{isRunning},
      player_{glm::vec2(windowWidth / 2.f, windowHeight / 2.f),
              windowWidth, windowHeight}
{
    logger_.Log("Command PlayState constructor called");
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
        if (event.type == SDL_KEYDOWN &&
            event.key.keysym.sym == SDLK_ESCAPE) {
            isRunning_ = false;
            return;
        }
        // Hand the event to the bindings; the state stays decoupled from keys.
        input_.handleInput(event);
    }
}

void PlayState::update() {
    int timeToWait = MILLISECS_PER_FRAME - (SDL_GetTicks() - millisecondsPreviousFrame_);
    if (timeToWait > 0 && timeToWait <= MILLISECS_PER_FRAME)
        SDL_Delay(timeToWait);
    millisecondsPreviousFrame_ = SDL_GetTicks();

    // The heart of the pattern: execute whatever command input is issuing,
    // without knowing what it is.
    if (Command *command = input_.getLastCommand())
        player_.move(command->getDirection());
}

void PlayState::render() {
    SDL_SetRenderDrawColor(renderer_, 24, 28, 38, 255);
    SDL_RenderClear(renderer_);

    glm::vec2 p = player_.position();
    SDL_Rect rect = {static_cast<int>(p.x), static_cast<int>(p.y),
                     player_.size(), player_.size()};
    SDL_SetRenderDrawColor(renderer_, 120, 200, 255, 255);
    SDL_RenderFillRect(renderer_, &rect);

    SDL_RenderPresent(renderer_);
}
