#include "playState.h"

#include <cmath>
#include <string>

#include "pixelText.h"    // shared: include/pixelText.h

const std::string PlayState::s_playID = "PLAY";

PlayState::PlayState(SDL_Renderer *renderer, int windowWidth, int windowHeight,
                     bool isDebugging, AssetStore_Ptr assetStore, bool &isRunning)
    : renderer_{renderer}, windowWidth_{windowWidth}, windowHeight_{windowHeight},
      isDebugging_{isDebugging}, assetStore_{std::move(assetStore)},
      isRunning_{isRunning}
{
    logger_.Log("PlayState constructor called");
    Rebuild(800);
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

// ONE builder for both layouts: the same particles, in the same order, differing
// only in how they are laid out. Two builders is how a comparison starts comparing
// two different worlds.
void PlayState::Rebuild(std::size_t count) {
    mixed_ = locality::MixedWorld{count};
    split_ = locality::SplitWorld{count};
    for (std::size_t i = 0; i < count; ++i) {
        const float f = static_cast<float>(i);
        const float x  = std::fmod(f * 7.13f, 96.0f) - 48.0f;
        const float y  = std::fmod(f * 3.71f, 96.0f) - 48.0f;
        const float vx = 0.31f * ((static_cast<int>(i) % 7) - 3);
        const float vy = 0.27f * ((static_cast<int>(i) % 5) - 2);
        const std::string name = "P" + std::to_string(i);
        mixed_.Set(i, x, y, vx, vy, name);
        split_.Set(i, x, y, vx, vy, name);
    }
    hasBenchmark_ = false;
    mixedNs_ = splitNs_ = 0.0;
    mixedSum_ = splitSum_ = 0.0;
    status_ = "READY";
}

void PlayState::Step(float dt) {
    // The demo drives its ANIMATION from the split layout alone -- the fast one.
    // The two are compared by the benchmark, not by every frame, so the screen is
    // not paying for the slow layout sixty times a second.
    split_.Step(dt, view_);
}

void PlayState::RunBenchmark() {
    using Clock = unsigned long long;
    const int kIterations = 12;
    const float dt = 1.0f / 60.0f;
    const double frequency = static_cast<double>(SDL_GetPerformanceFrequency());
    const std::size_t count = split_.size();

    // ⚠️ WARM BOTH UP FIRST. The first pass over a fresh array pays for cache misses
    // that are not the layout's fault but are the layout's to be blamed for if the
    // timing starts cold. Both get the same treatment, so the comparison is fair.
    mixed_.Step(dt, view_);
    split_.Step(dt, view_);

    const Clock startMixed = SDL_GetPerformanceCounter();
    for (int i = 0; i < kIterations; ++i)
        mixed_.Step(dt, view_);
    const Clock endMixed = SDL_GetPerformanceCounter();

    const Clock startSplit = SDL_GetPerformanceCounter();
    for (int i = 0; i < kIterations; ++i)
        split_.Step(dt, view_);
    const Clock endSplit = SDL_GetPerformanceCounter();

    // A CHECKSUM, so the benchmark reports more than a duration: if the two layouts
    // disagree, the "fast" one did different work, and that is the failure a timing
    // number cannot show.
    mixedSum_ = splitSum_ = 0.0;
    for (std::size_t i = 0; i < count; ++i) {
        mixedSum_ += mixed_.at(i).x;
        splitSum_ += split_.hotAt(i).x;
    }
    // Checked with a tolerance rather than exactly: both loops run the same
    // operations in the same order, but a compiler may vectorize one and contract a
    // multiply-add in the other, so the last bit can differ legitimately.
    const bool same = std::fabs(mixedSum_ - splitSum_) <= 1e-2;

    const double steps = static_cast<double>(kIterations) * static_cast<double>(count);
    mixedNs_ = static_cast<double>(endMixed - startMixed) / frequency * 1e9 / steps;
    splitNs_ = static_cast<double>(endSplit - startSplit) / frequency * 1e9 / steps;
    hasBenchmark_ = true;
    // ⚠️ THE STATUS LINE REPORTS THE METHOD, NOT THE VERDICT. It used to repeat
    // "SUMS MATCH", which the column beside the timings already says -- one fact
    // printed twice on one screen. How the number was obtained is the thing the
    // reader cannot see: warm-up passes, and how many timed ones.
    status_ = (same ? "TIMED " : "SUMS DIFFER AFTER ")
              + std::to_string(kIterations) + " PASSES OVER "
              + std::to_string(count) + " PARTICLES";
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
        case SDLK_1: Rebuild(200);  status_ = "200 PARTICLES"; break;
        case SDLK_2: Rebuild(2000); status_ = "2000 PARTICLES"; break;
        case SDLK_b: RunBenchmark(); break;
        case SDLK_r: Rebuild(split_.size()); break;
        default: break;
        }
    }
}

void PlayState::update() {
    int timeToWait = MILLISECS_PER_FRAME - (SDL_GetTicks() - millisecondsPreviousFrame_);
    if (timeToWait > 0 && timeToWait <= MILLISECS_PER_FRAME)
        SDL_Delay(timeToWait);
    millisecondsPreviousFrame_ = SDL_GetTicks();

    Step(1.0f / 60.0f);
}

namespace {

// One decimal place, and never a bare trailing dot. `std::to_string(x).substr(0, 4)`
// gave "14.6" on this machine and would have given "123." on a slow one (the demo
// builds at -O0), which reads as a broken screen rather than a large number.
std::string OneDecimal(double v) {
    const unsigned long long whole = static_cast<unsigned long long>(v);
    const unsigned long long tenth =
        static_cast<unsigned long long>((v - static_cast<double>(whole)) * 10.0 + 0.5);
    return std::to_string(whole) + "." + std::to_string(tenth % 10ULL);
}

const SDL_Color kInk    = {230, 230, 235, 255};
const SDL_Color kDim    = {140, 150, 165, 255};
const SDL_Color kRow    = {170, 180, 195, 255};
const SDL_Color kCursor = {255, 200,  80, 255};
const SDL_Color kField  = { 60,  70,  90, 255};
const SDL_Color kDot    = {120, 200, 255, 255};
const SDL_Color kHidden = { 70,  80, 100, 255};
} // namespace

// ── The field ────────────────────────────────────────────────────────────────
//
// ⚠️ AND DRAWING IS THE COST THE PATTERN ADDS, SHOWN RATHER THAN GLOSSED: the hot
// array has the positions and the cold array has the visibility, so every dot reads
// BOTH arrays through one shared index. That is the reunion the specs pin, and it is
// why a split layout is not simply "better" -- the loop got cheaper and the reader
// got an index to keep.
void PlayState::DrawField(int x, int y, int w, int h) const {
    SDL_SetRenderDrawColor(renderer_, kField.r, kField.g, kField.b, 255);
    SDL_Rect box{x, y, w, h};
    SDL_RenderDrawRect(renderer_, &box);

    const float spanX = view_.maxX - view_.minX;
    const float spanY = view_.maxY - view_.minY;
    for (std::size_t i = 0; i < split_.size(); ++i) {
        const locality::ParticleHot &hot = split_.hotAt(i);
        const locality::ParticleCold &cold = split_.coldAt(i);   // the second array
        const int px = x + 1 + static_cast<int>((hot.x - view_.minX) / spanX * (w - 2));
        const int py = y + 1 + static_cast<int>((hot.y - view_.minY) / spanY * (h - 2));
        if (cold.visible) {
            SDL_SetRenderDrawColor(renderer_, kDot.r, kDot.g, kDot.b, 255);
            SDL_Rect dot{px, py, 2, 2};
            SDL_RenderFillRect(renderer_, &dot);
        }
    }
}

void PlayState::DrawNumbers(int x, int y) const {
    SDL_SetRenderDrawColor(renderer_, kInk.r, kInk.g, kInk.b, 255);
    DrawPixelText(renderer_, "LAYOUT", x, y, 4);

    SDL_SetRenderDrawColor(renderer_, kRow.r, kRow.g, kRow.b, 255);
    DrawPixelText(renderer_, "PARTICLES " + std::to_string(split_.size()), x, y + 28, 3);

    // The pattern's claim in BYTES, which a spec asserts and the screen restates:
    // one struct is exactly its hot fields, and the mixed one carries cold data in
    // the same cache line.
    DrawPixelText(renderer_, "HOT BYTES " + std::to_string(sizeof(locality::ParticleHot)),
                  x, y + 48, 3);
    DrawPixelText(renderer_, "MIXED BYTES " + std::to_string(sizeof(locality::ParticleMixed)),
                  x, y + 66, 3);

    if (!hasBenchmark_) {
        SDL_SetRenderDrawColor(renderer_, kDim.r, kDim.g, kDim.b, 255);
        DrawPixelText(renderer_, "PRESS B TO TIME BOTH", x, y + 96, 3);
        return;
    }

    SDL_SetRenderDrawColor(renderer_, kRow.r, kRow.g, kRow.b, 255);
    DrawPixelText(renderer_, "MIXED " + OneDecimal(mixedNs_) + " NS", x, y + 96, 3);
    SDL_SetRenderDrawColor(renderer_, kCursor.r, kCursor.g, kCursor.b, 255);
    DrawPixelText(renderer_, "SPLIT " + OneDecimal(splitNs_) + " NS", x, y + 114, 3);

    SDL_SetRenderDrawColor(renderer_, kRow.r, kRow.g, kRow.b, 255);
    const bool faster = splitNs_ > 0.0 && splitNs_ < mixedNs_;
    const std::string ratio = (splitNs_ > 0.0 && mixedNs_ > 0.0)
        ? OneDecimal(faster ? mixedNs_ / splitNs_ : splitNs_ / mixedNs_)
        : "0.0";
    DrawPixelText(renderer_, std::string(faster ? "SPLIT X" : "MIXED X") + ratio, x, y + 138, 3);

    // The honesty line: a duration alone cannot show that the faster loop did the
    // same work. This is the checksum from the benchmark.
    SDL_SetRenderDrawColor(renderer_, kDim.r, kDim.g, kDim.b, 255);
    DrawPixelText(renderer_, std::fabs(mixedSum_ - splitSum_) <= 1e-2 ? "SUM MATCH" : "SUM DIFFER",
                  x, y + 162, 3);
}

void PlayState::DrawHint(int x, int y, int scale) const {
    SDL_SetRenderDrawColor(renderer_, kDim.r, kDim.g, kDim.b, 255);
    DrawPixelText(renderer_, "1-2 PARTICLES  B TIME  R RESET  ESC QUIT", x, y, scale);
}

void PlayState::render() {
    SDL_SetRenderDrawColor(renderer_, 16, 18, 24, 255);
    SDL_RenderClear(renderer_);

    const int pad = 16;
    SDL_SetRenderDrawColor(renderer_, kInk.r, kInk.g, kInk.b, 255);
    DrawPixelText(renderer_, "DATA LOCALITY", pad, pad, 4);

    SDL_SetRenderDrawColor(renderer_, kDim.r, kDim.g, kDim.b, 255);
    DrawPixelText(renderer_, "SAME WORK, TWO LAYOUTS", pad, pad + 28, 4);

    DrawField(pad, 96, 380, 240);
    DrawNumbers(416, 96);

    SDL_SetRenderDrawColor(renderer_, kInk.r, kInk.g, kInk.b, 255);
    DrawPixelText(renderer_, "STATUS " + status_, pad, 360, 3);

    DrawHint(pad, windowHeight_ - 32, 3);

    SDL_RenderPresent(renderer_);
}
