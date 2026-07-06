#pragma once

#include <SDL2/SDL.h>
#include <string>

#include <stormengine2/assetStore.h>
#include <stormengine2/logger.h>
#include <stormengine2/states/gameState.h>

#include "../loop/fixedTimestep.h"

// Game Loop demo: a ball is simulated at a *fixed* update rate (deliberately low,
// so steps are chunky) while the window renders at full frame rate. Rendering
// interpolates between the ball's previous and current fixed positions using the
// leftover-time `alpha`, so motion stays smooth even though the sim ticks slowly.
//
// Controls: Space toggle interpolation · Up/Down update rate · R reset · Esc quit
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

    void Reset();
    void SetRate(float hz);
    void FixedUpdate(); // advance the ball exactly one fixed step

    SDL_Renderer  *renderer_;
    int            windowWidth_, windowHeight_;
    bool           isDebugging_;
    AssetStore_Ptr assetStore_;
    Logger         logger_;
    bool          &isRunning_;

    FixedTimestep loop_{0.1f}; // 10 Hz default — chunky enough to see interpolation
    float updateHz_    = 10.f;
    bool  interpolate_ = true;

    float radius_ = 16.f;
    float prevX_ = 0.f, prevY_ = 0.f; // ball position last fixed step
    float curX_  = 0.f, curY_  = 0.f; // ball position this fixed step
    float velX_  = 0.f, velY_  = 0.f;
    float alpha_ = 0.f;               // this frame's render interpolation factor

    int millisecondsPreviousFrame_ = 0;
};
