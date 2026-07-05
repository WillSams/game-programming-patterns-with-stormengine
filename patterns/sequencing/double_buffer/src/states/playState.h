#pragma once

#include <SDL2/SDL.h>
#include <string>

#include <stormengine2/assetStore.h>
#include <stormengine2/logger.h>
#include <stormengine2/states/gameState.h>

#include "../life/life.h"
#include "../util/doubleBuffer.h"

// Conway's Game of Life, driven by the Double Buffer pattern. Each generation is
// computed by reading the current grid and writing the next one, then swapping —
// so neighbor reads always see a coherent, whole generation.
//
// Controls: Space pause/resume · N single-step (while paused) · R reseed · Esc quit
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

    void Seed();
    void Advance(); // Step(current -> next) then swap

    SDL_Renderer  *renderer_;
    int            windowWidth_, windowHeight_;
    bool           isDebugging_;
    AssetStore_Ptr assetStore_;
    Logger         logger_;
    bool          &isRunning_;

    DoubleBuffer<Grid> board_;
    int   cellSize_     = 8;
    int   cols_         = 0;
    int   rows_         = 0;
    bool  paused_       = false;
    float stepTimer_    = 0.f;
    float stepInterval_ = 0.08f; // seconds per generation

    int millisecondsPreviousFrame_ = 0;
};
