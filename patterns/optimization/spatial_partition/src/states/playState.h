#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <vector>

#include <stormengine2/assetStore.h>
#include <stormengine2/logger.h>
#include <stormengine2/states/gameState.h>

#include "../partition/grid.h"

// The demo: a field of drifting entities in a fixed 10x10 grid, with a probe cell
// that asks the partition "who is here" and a counter for what that question cost.
//
// ⚠️ THE CONTRAST IS THE WHOLE PICTURE. The probe's cell holds a handful of entities
// and those are the only ones a query touches; the field holds two hundred and
// forty. The bright dots are the candidates, the dim ones are entities the query
// never looked at, and the percentage is how much of the world was skipped.
//
// The entity positions live in NORMALIZED coordinates; the grid owns the bucketing
// and nothing else. The drawing maps one to the other, and the grid never learns
// that a screen exists.
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

    static constexpr int kCells     = 10;
    static constexpr int kEntities  = 240;

    void Rebuild();

    void DrawGrid(int x, int y, int w, int h) const;
    void DrawEntities(int x, int y, int w, int h);
    void DrawCandidates(int x, int y, int w, int h);
    void DrawProbe(int x, int y, int w, int h) const;
    void DrawNumbers(int x, int y);
    void DrawHint(int x, int y, int scale) const;

    SDL_Renderer  *renderer_;
    int            windowWidth_, windowHeight_;
    bool           isDebugging_;
    AssetStore_Ptr assetStore_;
    Logger         logger_;
    bool          &isRunning_;

    int millisecondsPreviousFrame_ = 0;

    partition::Grid   grid_{kCells, kEntities};
    std::vector<float> x_, y_, vx_, vy_;
    std::vector<int>   candidates_;      // refilled by the probe query each frame

    float phase_     = 0.0f;
    float probeX_    = 0.5f;
    float probeY_    = 0.5f;
    bool  autoMove_  = true;
    bool  probeMoves_ = true;
    bool  showGrid_  = true;
    std::string status_ = "AUTO MOVE ON";

    static constexpr float kDt = 1.0f / 60.0f;

    int FieldX(float nx, int x, int w) const;
    int FieldY(float ny, int y, int h) const;
};