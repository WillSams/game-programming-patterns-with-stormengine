#pragma once

#include <SDL2/SDL.h>
#include <string>

#include <stormengine2/assetStore.h>
#include <stormengine2/logger.h>
#include <stormengine2/states/gameState.h>

#include "../locality/particles.h"

// The demo of the pattern: one simulation, two layouts, and a timer.
//
// ⚠️ THE TIMER IS THE DEMO, NOT THE TEST. A spec cannot assert that memory is
// faster -- that is a fact about a machine -- so the suite pins that the two
// layouts agree and that the layout is what the pattern claims (see
// specs/dataLocality.spec.cpp). This screen shows the measurement, and it reports
// it honestly: it warms both layouts up first, and it shows a CHECKSUM so the two
// runs are known to have done the same work rather than merely the same loop count.
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

    void Rebuild(std::size_t count);
    void Step(float dt);
    void RunBenchmark();

    void DrawField(int x, int y, int w, int h) const;
    void DrawNumbers(int x, int y) const;
    void DrawHint(int x, int y, int scale) const;

    SDL_Renderer  *renderer_;
    int            windowWidth_, windowHeight_;
    bool           isDebugging_;
    AssetStore_Ptr assetStore_;
    Logger         logger_;
    bool          &isRunning_;

    int millisecondsPreviousFrame_ = 0;

    // Both layouts hold the same world, built by ONE builder, so a comparison
    // between them can only be about layout.
    locality::MixedWorld mixed_{0};
    locality::SplitWorld split_{0};
    locality::View       view_;

    double mixedNs_ = 0.0;      // measured ns per particle-step, or 0 = not run
    double splitNs_ = 0.0;
    double mixedSum_ = 0.0;     // the checksum each layout produced
    double splitSum_ = 0.0;
    bool   hasBenchmark_ = false;
    std::string status_ = "STEPPING BOTH LAYOUTS";
};
