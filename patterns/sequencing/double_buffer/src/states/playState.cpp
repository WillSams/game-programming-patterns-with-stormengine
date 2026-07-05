#include "playState.h"

#include <random>

const std::string PlayState::s_playID = "PLAY";

PlayState::PlayState(SDL_Renderer *renderer, int windowWidth, int windowHeight,
                     bool isDebugging, AssetStore_Ptr assetStore, bool &isRunning)
    : renderer_{renderer}, windowWidth_{windowWidth}, windowHeight_{windowHeight},
      isDebugging_{isDebugging}, assetStore_{std::move(assetStore)},
      isRunning_{isRunning}
{
    cols_  = windowWidth_  / cellSize_;
    rows_  = windowHeight_ / cellSize_;
    board_ = DoubleBuffer<Grid>(Grid(cols_, rows_));
    Seed();
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

// Randomly populate ~28% of the current buffer, then mirror it into the back
// buffer so a paused first frame shows the same state either way.
void PlayState::Seed() {
    static std::mt19937 rng{std::random_device{}()};
    std::bernoulli_distribution live(0.28);

    Grid &g = board_.current();
    for (int y = 0; y < g.height; ++y)
        for (int x = 0; x < g.width; ++x)
            g.set(x, y, live(rng));

    board_.next() = g;
    stepTimer_ = 0.f;
}

// The Double Buffer step: read the current generation, write the next, publish.
void PlayState::Advance() {
    Step(board_.current(), board_.next());
    board_.swap();
}

void PlayState::processInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) { isRunning_ = false; return; }
        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE: isRunning_ = false; return;
            case SDLK_SPACE:  paused_ = !paused_;     break;
            case SDLK_n:      if (paused_) Advance(); break;
            case SDLK_r:      Seed();                 break;
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
    millisecondsPreviousFrame_ = SDL_GetTicks();

    if (paused_) return;

    stepTimer_ += dt;
    if (stepTimer_ >= stepInterval_) {
        stepTimer_ -= stepInterval_;
        Advance();
    }
}

void PlayState::render() {
    SDL_SetRenderDrawColor(renderer_, 16, 18, 24, 255);
    SDL_RenderClear(renderer_);

    // Draw the stable, published buffer — never a half-computed generation.
    const Grid &g = board_.current();
    SDL_SetRenderDrawColor(renderer_, 120, 220, 160, 255);
    for (int y = 0; y < g.height; ++y)
        for (int x = 0; x < g.width; ++x)
            if (g.alive(x, y)) {
                SDL_Rect cell = { x * cellSize_, y * cellSize_,
                                  cellSize_ - 1, cellSize_ - 1 };
                SDL_RenderFillRect(renderer_, &cell);
            }

    SDL_RenderPresent(renderer_);
}
