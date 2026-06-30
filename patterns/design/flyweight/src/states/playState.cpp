#include "playState.h"

#include <random>

const std::string PlayState::s_playID = "PLAY";

PlayState::PlayState(SDL_Renderer *renderer, int windowWidth, int windowHeight,
                     bool isDebugging, AssetStore_Ptr assetStore, bool &isRunning)
    : renderer_{renderer}, windowWidth_{windowWidth}, windowHeight_{windowHeight},
      isDebugging_{isDebugging}, assetStore_{std::move(assetStore)},
      isRunning_{isRunning}
{
    world_.generateTerrain(std::random_device{}());

    // Each texture is loaded once and shared across every tile that uses it —
    // a flyweight of its own, on top of the shared Terrain instances.
    const std::string gfx = std::string(DATA_PREFIX) + "gfx/";
    assetStore_->AddTexture(renderer_, "grass", gfx + "grass.png");
    assetStore_->AddTexture(renderer_, "water", gfx + "water.png");
    assetStore_->AddTexture(renderer_, "hill",  gfx + "hill.png");

    logger_.Log("Flyweight PlayState constructor called");
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

    const int tile    = 48;
    const int gridW   = WIDTH * tile;
    const int gridH   = HEIGHT * tile;
    const int offsetX = (windowWidth_ - gridW) / 2;
    const int offsetY = (windowHeight_ - gridH) / 2;

    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            const Terrain &t = world_.getTile(x, y);
            SDL_Rect cell = {offsetX + x * tile, offsetY + y * tile, tile, tile};
            // The same shared texture is drawn for every tile of this terrain.
            SDL_Texture *tex = assetStore_->GetTexture(t.getTextureName());
            if (tex)
                SDL_RenderCopy(renderer_, tex, nullptr, &cell);
        }
    }

    SDL_RenderPresent(renderer_);
}
