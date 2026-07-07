#include "playState.h"

#include <memory>

#include "../entities/skeleton.h"
#include "../entities/statue.h"

using namespace um;

const std::string PlayState::s_playID = "PLAY";

PlayState::PlayState(SDL_Renderer *renderer, int windowWidth, int windowHeight,
                     bool isDebugging, AssetStore_Ptr assetStore, bool &isRunning)
    : renderer_{renderer}, windowWidth_{windowWidth}, windowHeight_{windowHeight},
      isDebugging_{isDebugging}, assetStore_{std::move(assetStore)},
      isRunning_{isRunning}
{
    BuildWorld();
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

void PlayState::BuildWorld() {
    const float left  = 40.f;
    const float right = windowWidth_ - 40.f;

    // Skeletons: each paces its lane at its own speed and heading.
    const float speeds[] = { 120.f, 190.f, 90.f, 240.f, 150.f };
    for (int i = 0; i < 5; ++i) {
        float y = 90.f + i * 60.f;
        float x = (i % 2 == 0) ? left : right;
        world_.add(std::make_unique<Skeleton>(x, y, left, right, speeds[i]));
    }

    // Statues: each shoots lightning on its own period.
    const float periods[] = { 0.6f, 0.9f, 1.3f, 1.7f };
    for (int i = 0; i < 4; ++i) {
        float x = 140.f + i * 170.f;
        world_.add(std::make_unique<Statue>(x, windowHeight_ - 110.f, periods[i]));
    }
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
    }
}

void PlayState::update() {
    int timeToWait = MILLISECS_PER_FRAME - (SDL_GetTicks() - millisecondsPreviousFrame_);
    if (timeToWait > 0 && timeToWait <= MILLISECS_PER_FRAME)
        SDL_Delay(timeToWait);

    float dt = (SDL_GetTicks() - millisecondsPreviousFrame_) / 1000.f;
    millisecondsPreviousFrame_ = SDL_GetTicks();

    // The Update Method: one loop drives every entity's own behavior.
    world_.update(dt);
}

void PlayState::render() {
    SDL_SetRenderDrawColor(renderer_, 16, 18, 24, 255);
    SDL_RenderClear(renderer_);

    const int half = 11; // half the entity square size
    for (auto &entity : world_) {
        Vec2 p = entity->position();
        SDL_Rect box = { static_cast<int>(p.x) - half, static_cast<int>(p.y) - half,
                         half * 2, half * 2 };

        if (auto *statue = dynamic_cast<Statue *>(entity.get())) {
            // Dim statue that flares bright right after shooting.
            float g = statue->glow();
            SDL_SetRenderDrawColor(renderer_,
                                   static_cast<Uint8>(90 + 165 * g),
                                   static_cast<Uint8>(70 + 120 * g),
                                   60, 255);
        } else {
            SDL_SetRenderDrawColor(renderer_, 120, 200, 255, 255); // skeleton
        }
        SDL_RenderFillRect(renderer_, &box);
    }

    SDL_RenderPresent(renderer_);
}
