#pragma once

#include <SDL2/SDL.h>
#include <string>

#include <stormengine2/assetStore.h>
#include <stormengine2/logger.h>
#include <stormengine2/states/gameState.h>

#include "../observer/ScoreSubject.h"
#include "../observer/HudObserver.h"
#include "../observer/MilestoneObserver.h"

// Demonstrates the Observer pattern: pressing Space adds points to a
// ScoreSubject, which notifies two independent observers — a HUD (draws the
// score bar) and a Milestone watcher (flashes the screen every 100 points).
// The subject never references either observer's behaviour.
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

    SDL_Renderer  *renderer_;
    int            windowWidth_, windowHeight_;
    bool           isDebugging_;
    AssetStore_Ptr assetStore_;
    Logger         logger_;
    bool          &isRunning_;

    ScoreSubject      subject_;
    HudObserver       hud_;
    MilestoneObserver milestone_{100};

    float flashTimer_ = 0.f; // seconds remaining on the milestone flash

    int millisecondsPreviousFrame_ = 0;
};
