#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <vector>

#include <stormengine2/assetStore.h>
#include <stormengine2/logger.h>
#include <stormengine2/states/gameState.h>

#include "../events/eventQueue.h"

// The Event Queue demo, and it shows the pattern as TWO COLUMNS OF THE SAME MOMENT:
// what the SENDER put in, and what the LISTENERS did about it. The two never
// mention each other -- the sender calls `send` and knows nothing more, which is
// the entire pattern, and it is visible here because the columns are produced by
// different code.
//
// The pattern lives in src/events/ (pure, spec'd); this screen is presentation.
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

    void Send(const events::Event &event);   // what a "system" does
    void DrawSent(int x, int y, int scale) const;
    void DrawListeners(int x, int y, int scale) const;
    void DrawReacted(int x, int y, int scale) const;
    void DrawHint(int x, int y, int scale) const;

    SDL_Renderer  *renderer_;
    int            windowWidth_, windowHeight_;
    bool           isDebugging_;
    AssetStore_Ptr assetStore_;
    Logger         logger_;
    bool          &isRunning_;

    int millisecondsPreviousFrame_ = 0;

    events::EventQueue queue_;
    std::vector<std::string> sent_;      // the SENDER's view
    std::vector<std::string> reacted_;   // the LISTENERS' view
    std::size_t lastDrained_ = 0;
    bool soundListener_ = true;          // listeners can be switched off without
    bool achievementListener_ = true;    // the sender ever knowing they exist
    std::string status_ = "READY";
};
