#pragma once

#include <SDL2/SDL.h>
#include <memory>
#include <string>
#include <vector>

#include <stormengine2/assetStore.h>
#include <stormengine2/logger.h>
#include <stormengine2/states/gameState.h>

#include "../powers/powers.h"

// The Subclass Sandbox demo: pick a power and watch what it says to the sandbox.
// The pattern lives in src/powers/ (pure, spec'd); this screen is presentation --
// the power list, the effect log the power produced, and the world it acted on.
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

    void Activate(std::size_t index);
    void DrawPowers(int x, int y, int scale) const;
    void DrawEffects(int x, int y, int scale) const;
    void DrawWorld(int x, int y, int scale) const;
    void DrawHint(int x, int y, int scale) const;

    SDL_Renderer  *renderer_;
    int            windowWidth_, windowHeight_;
    bool           isDebugging_;
    AssetStore_Ptr assetStore_;
    Logger         logger_;
    bool          &isRunning_;

    int millisecondsPreviousFrame_ = 0;

    // The powers are the same four every run; the sandbox is the world they act
    // on, and `shown_` is the log of the LAST activation so the panel reads as
    // "what this power did" rather than a growing pile.
    std::vector<std::unique_ptr<powers::Superpower>> powers_;
    powers::Sandbox world_;
    std::vector<powers::Effect> shown_;
    std::size_t index_ = 0;
    std::string status_ = "READY";
};
