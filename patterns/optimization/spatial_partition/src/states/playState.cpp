#include "playState.h"

#include <cmath>

#include "pixelText.h"    // shared: include/pixelText.h

const std::string PlayState::s_playID = "PLAY";

namespace {
// A drifting field, not a physics sim: every entity keeps a constant velocity and
// bounces off the walls. What matters is that they CROSS cell boundaries, because a
// crossing is the only thing the partition has to work for.
constexpr float kSpeedMin = 0.010f;    // normalized units per frame
constexpr float kSpeedMax = 0.030f;
} // namespace

PlayState::PlayState(SDL_Renderer *renderer, int windowWidth, int windowHeight,
                     bool isDebugging, AssetStore_Ptr assetStore, bool &isRunning)
    : renderer_{renderer}, windowWidth_{windowWidth}, windowHeight_{windowHeight},
      isDebugging_{isDebugging}, assetStore_{std::move(assetStore)},
      isRunning_{isRunning}
{
    logger_.Log("PlayState constructor called");
    Rebuild();
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

// Deterministic placement and velocities: a fixed-spacing sweep with a fixed
// velocity pattern, so two runs of the demo look the same and a screenshot is
// comparable. A real game would randomize; a teaching demo wants reproducibility.
void PlayState::Rebuild() {
    grid_ = partition::Grid(kCells, kEntities);
    x_.assign(kEntities, 0.0f);
    y_.assign(kEntities, 0.0f);
    vx_.assign(kEntities, 0.0f);
    vy_.assign(kEntities, 0.0f);
    candidates_.clear();
    phase_ = 0.0f;

    for (int i = 0; i < kEntities; ++i) {
        x_[i] = 0.02f + 0.96f * static_cast<float>(i % 24) / 23.0f;
        y_[i] = 0.02f + 0.96f * static_cast<float>((i / 24) % 10) / 9.0f;
        const float s = kSpeedMin + (kSpeedMax - kSpeedMin) * static_cast<float>(i % 7) / 6.0f;
        vx_[i] = ((i % 2) ? 1.0f : -1.0f) * s;
        vy_[i] = ((i % 3) ? 1.0f : -1.0f) * s * 0.7f;
        // ⚠️ EVERY ENTITY IS INSERTED WHERE IT WILL BE DRAWN. A partition whose data is
        // seeded from one place and drawn from another looks right and answers every
        // query about the wrong position.
        grid_.Insert(i, x_[i], y_[i]);
    }
    status_ = autoMove_ ? "AUTO MOVE ON" : "AUTO MOVE OFF";
}

void PlayState::update() {
    int timeToWait = MILLISECS_PER_FRAME - (SDL_GetTicks() - millisecondsPreviousFrame_);
    if (timeToWait > 0 && timeToWait <= MILLISECS_PER_FRAME)
        SDL_Delay(timeToWait);
    millisecondsPreviousFrame_ = SDL_GetTicks();

    phase_ += kDt;

    // ⚠️ THE MOVE AND THE BUCKET UPDATE ARE THE SAME STATEMENT. A demo that dragged
    // the entities around without telling the grid would look perfect and query a
    // world that no longer exists -- the stale-bucket bug, on screen.
    if (autoMove_) {
        for (int i = 0; i < kEntities; ++i) {
            x_[i] += vx_[i];
            y_[i] += vy_[i];
            if (x_[i] < 0.0f) { x_[i] = -x_[i]; vx_[i] = -vx_[i]; }
            if (x_[i] >= 1.0f) { x_[i] = 2.0f - x_[i]; vx_[i] = -vx_[i]; }
            if (y_[i] < 0.0f) { y_[i] = -y_[i]; vy_[i] = -vy_[i]; }
            if (y_[i] >= 1.0f) { y_[i] = 2.0f - y_[i]; vy_[i] = -vy_[i]; }
            // ⚠️ THE BOUNCE CAN LAND EXACTLY ON 1.0, AND 1.0 IS NOT IN THE WORLD. The
            // reflection `2 - x` maps 1.0 to 1.0, so an entity that hit the wall
            // exactly would be drawn on the box edge and refused by `Move` -- outside
            // the data structure it is drawn from, for a frame. Clamping the result
            // into [0,1) keeps the picture and the buckets describing the same world.
            if (x_[i] >= 1.0f) x_[i] = 0.99999f;
            if (y_[i] >= 1.0f) y_[i] = 0.99999f;
            grid_.Move(i, x_[i], y_[i]);
        }
    }

    if (probeMoves_) {
        probeX_ = 0.5f + 0.40f * std::sin(phase_ * 0.7f);
        probeY_ = 0.5f + 0.40f * std::sin(phase_ * 1.1f);
    }
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
        case SDLK_a:
            autoMove_ = !autoMove_;
            status_ = autoMove_ ? "AUTO MOVE ON" : "AUTO MOVE OFF -- THE GRID IS FROZEN";
            break;
        case SDLK_p:
            probeMoves_ = !probeMoves_;
            status_ = probeMoves_ ? "PROBE MOVING" : "PROBE PARKED";
            break;
        case SDLK_g:
            showGrid_ = !showGrid_;
            status_ = showGrid_ ? "GRID SHOWN" : "GRID HIDDEN -- THE BUCKETS ARE STILL THERE";
            break;
        case SDLK_r:
            grid_.ResetCounters();
            status_ = "COUNTERS CLEARED -- THE WORLD IS UNCHANGED";
            break;
        case SDLK_0:
            Rebuild();
            status_ = "WORLD REBUILT -- CAPACITY IS STILL " + std::to_string(kEntities);
            break;
        default: break;
        }
    }
}

int PlayState::FieldX(float nx, int x, int w) const {
    return x + 1 + static_cast<int>(nx * static_cast<float>(w - 2));
}
int PlayState::FieldY(float ny, int y, int h) const {
    return y + 1 + static_cast<int>(ny * static_cast<float>(h - 2));
}

void PlayState::DrawGrid(int x, int y, int w, int h) const {
    SDL_SetRenderDrawColor(renderer_, 45, 52, 68, 255);
    SDL_Rect box{x, y, w, h};
    SDL_RenderDrawRect(renderer_, &box);
    if (!showGrid_)
        return;
    for (int c = 1; c < kCells; ++c) {
        const int gx = x + c * w / kCells;
        const int gy = y + c * h / kCells;
        SDL_RenderDrawLine(renderer_, gx, y, gx, y + h);
        SDL_RenderDrawLine(renderer_, x, gy, x + w, gy);
    }
}

void PlayState::DrawEntities(int x, int y, int w, int h) {
    SDL_SetRenderDrawColor(renderer_, 70, 82, 105, 255);
    for (int i = 0; i < kEntities; ++i) {
        SDL_Rect dot{FieldX(x_[i], x, w) - 1, FieldY(y_[i], y, h) - 1, 2, 2};
        SDL_RenderFillRect(renderer_, &dot);
    }
}

// The bright dots -- and they are exactly the list the partition returned, not a
// re-derived "close enough" set. Drawing a different set from the one the counter
// counted is how a demo lies about its own data structure.
void PlayState::DrawCandidates(int x, int y, int w, int h) {
    SDL_SetRenderDrawColor(renderer_, 255, 200, 80, 255);
    for (int id : candidates_) {
        SDL_Rect dot{FieldX(x_[id], x, w) - 2, FieldY(y_[id], y, h) - 2, 5, 5};
        SDL_RenderFillRect(renderer_, &dot);
    }
}

void PlayState::DrawProbe(int x, int y, int w, int h) const {
    const int cell = grid_.CellIndex(probeX_, probeY_);
    if (cell < 0)
        return;
    const int cx = cell % kCells;
    const int cy = cell / kCells;
    const int left = x + cx * w / kCells;
    const int top  = y + cy * h / kCells;
    SDL_SetRenderDrawColor(renderer_, 120, 210, 255, 255);
    SDL_Rect marker{left, top, w / kCells, h / kCells};
    SDL_RenderDrawRect(renderer_, &marker);
}

void PlayState::DrawNumbers(int x, int y) {
    SDL_SetRenderDrawColor(renderer_, 230, 230, 235, 255);
    DrawPixelText(renderer_, "WORLD", x, y, 4);

    SDL_SetRenderDrawColor(renderer_, 170, 180, 195, 255);
    DrawPixelText(renderer_, "ENTITIES " + std::to_string(kEntities), x, y + 28, 3);
    DrawPixelText(renderer_, "GRID " + std::to_string(kCells) + " X "
                  + std::to_string(kCells), x, y + 46, 3);

    const int cell = grid_.CellIndex(probeX_, probeY_);
    const int cx = cell >= 0 ? cell % kCells : -1;
    const int cy = cell >= 0 ? cell / kCells : -1;
    DrawPixelText(renderer_, "PROBE " + std::to_string(cx) + "," + std::to_string(cy),
                  x, y + 70, 3);

    // ⚠️ THE TWO NUMBERS THE PATTERN EXISTS TO SEPARATE. `IN CELL` is what a query
    // touches; `ALL` is what the same query costs without a partition. The percentage
    // between them is the whole chapter.
    SDL_SetRenderDrawColor(renderer_, 255, 200, 80, 255);
    DrawPixelText(renderer_, "IN CELL " + std::to_string(candidates_.size()), x, y + 88, 3);
    SDL_SetRenderDrawColor(renderer_, 170, 180, 195, 255);
    DrawPixelText(renderer_, "ALL " + std::to_string(kEntities), x, y + 106, 3);

    // Rounded, not truncated: 2 candidates out of 240 truncated to "SKIPPED 100 PCT",
    // which reads as "nothing was examined" when two things were.
    const int pct = kEntities == 0 ? 0
        : 100 - (static_cast<int>(candidates_.size()) * 100 + kEntities / 2) / kEntities;
    SDL_SetRenderDrawColor(renderer_, 120, 210, 255, 255);
    DrawPixelText(renderer_, "SKIPPED " + std::to_string(pct) + " PCT", x, y + 124, 3);

    // The partition's own bill: every position update is a Move, and only a crossing
    // does bucket work.
    SDL_SetRenderDrawColor(renderer_, 170, 180, 195, 255);
    DrawPixelText(renderer_, "MOVES " + std::to_string(grid_.Moves()), x, y + 148, 3);
    DrawPixelText(renderer_, "CROSSINGS " + std::to_string(grid_.Crossings()), x, y + 166, 3);
}

void PlayState::DrawHint(int x, int y, int scale) const {
    SDL_SetRenderDrawColor(renderer_, 140, 150, 165, 255);
    DrawPixelText(renderer_, "A MOVE  P PROBE  G GRID  R CLEAR  0 REBUILD  ESC QUIT",
                  x, y, scale);
}

void PlayState::render() {
    SDL_SetRenderDrawColor(renderer_, 16, 18, 24, 255);
    SDL_RenderClear(renderer_);

    const int pad = 16;
    SDL_SetRenderDrawColor(renderer_, 230, 230, 235, 255);
    DrawPixelText(renderer_, "SPATIAL PARTITION", pad, pad, 4);

    SDL_SetRenderDrawColor(renderer_, 140, 150, 165, 255);
    DrawPixelText(renderer_, "ASK THE CELL, NOT THE WORLD", pad, pad + 28, 4);

    // ⚠️ THE QUERY RUNS BEFORE THE DRAW, and the drawing uses ITS RESULT. Refilling
    // `candidates_` inside the draw would count one list and show another.
    grid_.CandidatesAround(probeX_, probeY_, 0, candidates_);

    const int fx = pad, fy = 96, fw = 380, fh = 240;
    DrawGrid(fx, fy, fw, fh);
    DrawEntities(fx, fy, fw, fh);
    DrawCandidates(fx, fy, fw, fh);
    DrawProbe(fx, fy, fw, fh);
    DrawNumbers(416, 96);

    SDL_SetRenderDrawColor(renderer_, 230, 230, 235, 255);
    DrawPixelText(renderer_, "STATUS " + status_, pad, 360, 3);

    DrawHint(pad, windowHeight_ - 32, 3);

    SDL_RenderPresent(renderer_);
}