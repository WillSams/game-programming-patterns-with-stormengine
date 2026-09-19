#pragma once

#include <SDL2/SDL.h>
#include <string>

#include <stormengine2/assetStore.h>
#include <stormengine2/logger.h>
#include <stormengine2/states/gameState.h>

#include "../pool/particlePool.h"

// The demo: a particle fountain drawn from a pool of fixed size.
//
// ⚠️ THE THING TO WATCH IS THE RIGHT COLUMN, NOT THE FOUNTAIN. `CAPACITY` never
// moves and `CREATED` climbs forever, which is the entire promise of the pattern:
// the burst that is on screen has been handed out hundreds of times and allocated
// once. `REUSED` is the share of takes that came from a slot that had a previous
// life, and `REFUSED` is the pool saying no -- press B faster than particles expire
// and it climbs, and the fountain does not grow to accommodate you.
//
// It also stays honest about the cost: the pool walks its full capacity every frame,
// live or dead, so `ALIVE` climbing to `CAPACITY` is not what slows this down. The
// fixed walk is the trade.
class PlayState : public GameState {
public:
    PlayState(SDL_Renderer *renderer, int windowWidth, int windowHeight,
              bool isDebugging, AssetStore_Ptr assetStore, bool &isRunning);
    ~PlayState();

    void processInput() override;
    void update()       override;
    void render()       override;
    bool onEnter()      override;
    bool onExit()       override;
    std::string getStateID() const override { return s_playID; }

private:
    static const std::string s_playID;

    static constexpr std::size_t kCapacity = 120;

    void Spawn(int count);
    void Rebuild();

    void DrawParticles(int x, int y, int w, int h);
    void DrawSlots(int x, int y, int w, int h) const;
    void DrawNumbers(int x, int y) const;
    void DrawHint(int x, int y, int scale) const;

    SDL_Renderer  *renderer_;
    int            windowWidth_, windowHeight_;
    bool           isDebugging_;
    AssetStore_Ptr assetStore_;
    Logger         logger_;
    bool          &isRunning_;

    int millisecondsPreviousFrame_ = 0;

    pool::ParticlePool pool_{kCapacity};
    unsigned int       rng_      = 1u;    // one xorshift, so a fountain is repeatable
    bool               autoSpawn_ = true;
    float              spawnTimer_ = 0.0f;
    std::string        status_    = "AUTO ON -- WATCH CREATED";

    float NextFloat();                     // [0,1)
};