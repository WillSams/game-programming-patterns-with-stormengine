#include "playState.h"

#include "pixelText.h"    // shared: include/pixelText.h

const std::string PlayState::s_playID = "PLAY";

namespace {
// Particles live in NORMALIZED field coordinates -- x and y in [0,1] -- and are
// mapped to the box at draw time. Nothing here knows the window size.
constexpr float kGravity    = 0.5f;    // world units per second squared
constexpr float kSpawnY     = 0.95f;
constexpr float kAutoEvery  = 0.03f;   // seconds between fountain particles
constexpr float kDt         = 1.0f / 60.0f;

// ⚠️ THE LIFE IS DERIVED FROM THE LAUNCH SPEED, NOT PICKED. A projectile starting at
// kSpawnY returns to kSpawnY at `2 * speed / gravity`; giving it exactly that life
// means every particle is released at the bottom of its arc, inside the box. An
// independent life (the first version used a flat 1.4s) let particles fall past the
// bottom edge and draw over the occupancy strip below the field -- outside its box,
// which is a drawing bug rather than a simulation result.
float LifeForSpeed(float speed) { return 2.0f * speed / kGravity; }
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

// A tiny xorshift rather than <random>: the fountain is cosmetic, and a deterministic
// one means two runs of the demo look the same and a screenshot can be compared.
float PlayState::NextFloat() {
    rng_ ^= rng_ << 13;
    rng_ ^= rng_ >> 17;
    rng_ ^= rng_ << 5;
    return static_cast<float>(rng_ % 10000u) / 10000.0f;
}

void PlayState::Rebuild() {
    pool_.Clear();
    pool_.ResetCounters();
    rng_       = 1u;
    spawnTimer_ = 0.0f;
    status_    = autoSpawn_ ? "AUTO ON -- WATCH CREATED" : "AUTO OFF";
}

// ⚠️ ONE TAKE PER PARTICLE, AND THE REFUSAL IS PROPAGATED, NOT IGNORED. When the pool
// is full `Create` returns -1; the burst below simply spawns fewer. That is the whole
// reason the pool is fixed -- a spawn that cannot be satisfied is not allowed to
// quietly recycle a live particle, so the demo shows the shortfall in REFUSED.
void PlayState::Spawn(int count) {
    int spawned = 0;
    for (int i = 0; i < count; ++i) {
        // ⚠️ THE CURTAIN RISES, IT DOES NOT ARC ACROSS. The launch is straight up from
        // a random x. A single spawn point with a sideways velocity was the first
        // version: it stayed inside the box, but its life was under a second, so the
        // pool never held a crowd and switching AUTO off blanked the field before
        // REUSED had moved. Longer arcs need a small horizontal component to stay
        // inside, and a vertical launch from a spread of x is the one that can afford
        // them. A simulation that leaves the box it is drawn in is a lying picture;
        // this spread is what keeps it honest as the arcs got slower.
        const float x     = 0.12f + 0.76f * NextFloat();
        const float speed = 0.50f + 0.35f * NextFloat();     // upward launch
        const int slot = pool_.Create(x, kSpawnY, 0.0f, -speed,
                                      LifeForSpeed(speed));
        if (slot >= 0)
            ++spawned;
    }
    if (spawned == 0)
        status_ = "POOL FULL -- SPAWN REFUSED, ALIVE HELD";
    else if (spawned < count)
        status_ = "POOL FILLING -- " + std::to_string(spawned) + " OF "
                + std::to_string(count) + " SPAWNED";
}

void PlayState::update() {
    int timeToWait = MILLISECS_PER_FRAME - (SDL_GetTicks() - millisecondsPreviousFrame_);
    if (timeToWait > 0 && timeToWait <= MILLISECS_PER_FRAME)
        SDL_Delay(timeToWait);
    millisecondsPreviousFrame_ = SDL_GetTicks();

    // ⚠️ THE POOL WALKS ALL 120 SLOTS EVERY FRAME, dead ones included. A pool does not
    // get cheaper as it empties; it gets predictable. That predictability is what it
    // sells -- and why the capacity has to be right before the first frame.
    //
    // ⚠️ AND THE GRAVITY IS APPLIED HERE, NOT IN THE POOL. The pool integrates the
    // velocity it is given and owns the particle's life; acceleration is the caller's,
    // which is the only reason a generic pool can serve a fountain and a bullet
    // spread. It is also the bug this demo shipped first: it declared `kGravity`,
    // never applied it, and drew straight lines out through the top of the field.
    for (std::size_t i = 0; i < pool_.Capacity(); ++i) {
        const int idx = static_cast<int>(i);
        if (pool_.IsAlive(idx))
            pool_.At(idx).vy += kGravity * kDt;
    }
    pool_.Animate(kDt);

    if (autoSpawn_) {
        spawnTimer_ += kDt;
        while (spawnTimer_ >= kAutoEvery) {
            spawnTimer_ -= kAutoEvery;
            Spawn(1);
        }
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
        case SDLK_SPACE:
            Spawn(1);
            status_ = "ONE SPAWN -- REUSED " + std::to_string(pool_.Reused())
                    + " OF " + std::to_string(pool_.Created());
            break;
        case SDLK_b:
            Spawn(20);
            status_ = "BURST OF 20 -- POOL CAPPED AT "
                    + std::to_string(pool_.Capacity());
            break;
        case SDLK_a:
            autoSpawn_ = !autoSpawn_;
            status_ = autoSpawn_ ? "AUTO ON" : "AUTO OFF -- CREATED STOPS, SCENE KEEPS DRAWING";
            break;
        case SDLK_r:
            pool_.ResetCounters();
            status_ = "COUNTERS CLEARED -- THE POOL IS UNCHANGED";
            break;
        case SDLK_0:
            Rebuild();
            status_ = "POOL EMPTIED -- CAPACITY IS STILL "
                    + std::to_string(pool_.Capacity());
            break;
        default: break;
        }
    }
}

// ── The field ────────────────────────────────────────────────────────────────
void PlayState::DrawParticles(int x, int y, int w, int h) {
    for (std::size_t i = 0; i < pool_.Capacity(); ++i) {
        const int idx = static_cast<int>(i);
        if (!pool_.IsAlive(idx))
            continue;
        const pool::Particle &p = pool_.At(idx);
        const int px = x + static_cast<int>(p.x * (w - 2));
        const int py = y + static_cast<int>(p.y * (h - 2));

        // The SIZE carries the remaining life, since a filled rect has no alpha here.
        // The pool owns position and life; the drawing derives the rest.
        const int a = static_cast<int>(255 * p.Alpha());
        const int size = 2 + a / 100;              // 2..4 px, never zero
        SDL_SetRenderDrawColor(renderer_, 255, 190, 90, 255);
        SDL_Rect dot{px - size / 2, py - size / 2, size, size};
        SDL_RenderFillRect(renderer_, &dot);
    }
}

// Every slot, alive or free, as one small cell. This is the pool as a shape: the row
// never grows, and the amber cells move around inside it as particles are taken and
// handed back.
void PlayState::DrawSlots(int x, int y, int w, int h) const {
    const std::size_t n = pool_.Capacity();
    if (n == 0)
        return;
    for (std::size_t i = 0; i < n; ++i) {
        const int idx = static_cast<int>(i);
        if (pool_.IsAlive(idx))
            SDL_SetRenderDrawColor(renderer_, 255, 190, 90, 255);
        else
            SDL_SetRenderDrawColor(renderer_, 40, 46, 60, 255);
        // Exact edges, so the last cell lands on the box edge instead of leaving the
        // remainder of the integer division as a gap at the right.
        const int left  = x + static_cast<int>(i) * w / static_cast<int>(n);
        const int right = x + static_cast<int>(i + 1) * w / static_cast<int>(n);
        SDL_Rect box{left, y, right - left > 1 ? right - left - 1 : 1, h};
        SDL_RenderFillRect(renderer_, &box);
    }
}

void PlayState::DrawNumbers(int x, int y) const {
    SDL_SetRenderDrawColor(renderer_, 230, 230, 235, 255);
    DrawPixelText(renderer_, "POOL", x, y, 4);

    SDL_SetRenderDrawColor(renderer_, 170, 180, 195, 255);
    DrawPixelText(renderer_, "CAPACITY " + std::to_string(pool_.Capacity()), x, y + 28, 3);
    DrawPixelText(renderer_, "ALIVE " + std::to_string(pool_.AliveCount()), x, y + 46, 3);
    DrawPixelText(renderer_, "FREE " + std::to_string(pool_.FreeCount()), x, y + 64, 3);

    // ⚠️ CREATED IS THE ONE THAT KEEPS CLIMBING WHILE NOTHING IS ALLOCATED. Every
    // take is counted here, and every one after the first pass through the slots is
    // also a REUSE -- the pair is the pattern, stated as two numbers.
    SDL_SetRenderDrawColor(renderer_, 170, 180, 195, 255);
    DrawPixelText(renderer_, "CREATED " + std::to_string(pool_.Created()), x, y + 88, 3);

    SDL_SetRenderDrawColor(renderer_, 120, 210, 255, 255);
    DrawPixelText(renderer_, "REUSED " + std::to_string(pool_.Reused()), x, y + 106, 3);

    SDL_SetRenderDrawColor(renderer_, 255, 200, 80, 255);
    DrawPixelText(renderer_, "REFUSED " + std::to_string(pool_.Refused()), x, y + 124, 3);

    const std::size_t created = pool_.Created();
    const int pct = created == 0 ? 0
                                : static_cast<int>((pool_.Reused() * 100) / created);
    SDL_SetRenderDrawColor(renderer_, 120, 210, 255, 255);
    DrawPixelText(renderer_, "REUSE " + std::to_string(pct) + " PCT", x, y + 148, 3);
}

void PlayState::DrawHint(int x, int y, int scale) const {
    SDL_SetRenderDrawColor(renderer_, 140, 150, 165, 255);
    DrawPixelText(renderer_, "SPACE ONE  B BURST  A AUTO  R CLEAR  0 EMPTY  ESC QUIT",
                  x, y, scale);
}

void PlayState::render() {
    SDL_SetRenderDrawColor(renderer_, 16, 18, 24, 255);
    SDL_RenderClear(renderer_);

    const int pad = 16;
    SDL_SetRenderDrawColor(renderer_, 230, 230, 235, 255);
    DrawPixelText(renderer_, "OBJECT POOL", pad, pad, 4);

    SDL_SetRenderDrawColor(renderer_, 140, 150, 165, 255);
    DrawPixelText(renderer_, "ALLOCATE ONCE, HAND OUT THE FREE SLOTS", pad, pad + 28, 4);

    const int fx = pad, fy = 96, fw = 380, fh = 240;
    SDL_SetRenderDrawColor(renderer_, 60, 70, 90, 255);
    SDL_Rect field{fx, fy, fw, fh};
    SDL_RenderDrawRect(renderer_, &field);

    DrawParticles(fx, fy, fw, fh);
    DrawSlots(fx, fy + fh + 8, fw, 8);
    DrawNumbers(416, 96);

    SDL_SetRenderDrawColor(renderer_, 230, 230, 235, 255);
    DrawPixelText(renderer_, "STATUS " + status_, pad, 360, 3);

    DrawHint(pad, windowHeight_ - 32, 3);

    SDL_RenderPresent(renderer_);
}