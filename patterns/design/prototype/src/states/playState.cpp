#include "playState.h"

#include <algorithm>
#include <cmath>

#include "../monsters/Ghost.h"
#include "../monsters/Demon.h"
#include "../monsters/Sorcerer.h"

const std::string PlayState::s_playID = "PLAY";

PlayState::PlayState(SDL_Renderer *renderer, int windowWidth, int windowHeight,
                     bool isDebugging, AssetStore_Ptr assetStore, bool &isRunning)
    : renderer_{renderer}, windowWidth_{windowWidth}, windowHeight_{windowHeight},
      isDebugging_{isDebugging}, assetStore_{std::move(assetStore)},
      isRunning_{isRunning}, spawner_{std::make_unique<Ghost>()},
      rng_{std::random_device{}()}
{
    logger_.Log("Prototype PlayState constructor called");
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

void PlayState::spawnOne() {
    // The pattern in one line: clone the current prototype.
    std::unique_ptr<Monster> m = spawner_.spawn();

    std::uniform_real_distribution<float> px(40.f, windowWidth_ - 40.f);
    std::uniform_real_distribution<float> py(40.f, windowHeight_ - 40.f);
    std::uniform_real_distribution<float> angle(0.f, 6.2831853f);

    float a     = angle(rng_);
    float speed = m->speed();
    monsters_.push_back({std::move(m),
                         {px(rng_), py(rng_)},
                         {std::cos(a) * speed, std::sin(a) * speed}});
}

void PlayState::processInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) { isRunning_ = false; return; }
        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE: isRunning_ = false; return;
            // Swap the spawner's prototype and immediately clone one of it.
            case SDLK_1: spawner_.setPrototype(std::make_unique<Ghost>());    spawnOne(); break;
            case SDLK_2: spawner_.setPrototype(std::make_unique<Demon>());    spawnOne(); break;
            case SDLK_3: spawner_.setPrototype(std::make_unique<Sorcerer>()); spawnOne(); break;
            // Clone another of whatever the prototype currently is.
            case SDLK_SPACE: spawnOne(); break;
            default: break;
            }
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

    const float size = 24.f;
    for (auto &a : monsters_) {
        a.pos += a.vel * dt;
        if (a.pos.x < 0.f)                 { a.pos.x = 0.f;                 a.vel.x = -a.vel.x; }
        if (a.pos.x > windowWidth_ - size) { a.pos.x = windowWidth_ - size; a.vel.x = -a.vel.x; }
        if (a.pos.y < 0.f)                 { a.pos.y = 0.f;                 a.vel.y = -a.vel.y; }
        if (a.pos.y > windowHeight_ - size){ a.pos.y = windowHeight_ - size;a.vel.y = -a.vel.y; }
    }
}

void PlayState::render() {
    SDL_SetRenderDrawColor(renderer_, 24, 28, 38, 255);
    SDL_RenderClear(renderer_);

    const int size = 24;
    for (const auto &a : monsters_) {
        Color c = a.monster->color();
        SDL_SetRenderDrawColor(renderer_, c.r, c.g, c.b, 255);
        SDL_Rect rect = {static_cast<int>(a.pos.x), static_cast<int>(a.pos.y), size, size};
        SDL_RenderFillRect(renderer_, &rect);
    }

    // Swatch of the current prototype (what Space will clone).
    Color p = spawner_.prototype().color();
    SDL_SetRenderDrawColor(renderer_, p.r, p.g, p.b, 255);
    SDL_Rect proto = {16, 16, 32, 32};
    SDL_RenderFillRect(renderer_, &proto);
    SDL_SetRenderDrawColor(renderer_, 230, 230, 230, 255);
    SDL_RenderDrawRect(renderer_, &proto);

    SDL_RenderPresent(renderer_);
}
