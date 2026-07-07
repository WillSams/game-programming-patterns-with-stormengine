#pragma once

#include <SDL2/SDL.h>
#include <string>

#include <stormengine2/assetStore.h>
#include <stormengine2/logger.h>
#include <stormengine2/states/gameState.h>

#include "../entities/world.h"

// Update Method demo: a World of heterogeneous entities — skeletons pacing at
// their own speeds and statues shooting on their own periods — all advanced by a
// single `world.update(dt)` each frame. Each entity minds its own behavior.
//
// Controls: Esc quit
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

    void BuildWorld();

    SDL_Renderer  *renderer_;
    int            windowWidth_, windowHeight_;
    bool           isDebugging_;
    AssetStore_Ptr assetStore_;
    Logger         logger_;
    bool          &isRunning_;

    um::World world_;

    int millisecondsPreviousFrame_ = 0;
};
