#pragma once

#include <SDL2/SDL.h>
#include <string>

#include <stormengine2/assetStore.h>
#include <stormengine2/logger.h>
#include <stormengine2/states/gameState.h>

#include "../dirty/transformTree.h"

// The demo: a small scene graph drawn from its WORLD positions, with the work the
// dirty flag deferred shown as a counter beside it.
//
// ⚠️ THE PUNCHLINE IS THE TWO COUNTERS DIVERGING. Leave auto-move on and every frame
// dirties the tree, so recomputes track reads. Turn it OFF and the scene keeps
// being drawn -- READS keeps climbing -- while RECOMPUTES STOPS. That gap is the
// pattern: drawing a frame that has not changed costs no geometry.
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

    void Build();
    // ⚠️ NOT `const`, AND THAT IS THE PATTERN SHOWING THROUGH: drawing is the READ
    // that resolves the stale nodes, and a read of a deferred value MUTATES the cache
    // and the counters. A const version would need a `const_cast` (it had one) to do
    // the one thing this pattern is for.
    void DrawTree(int x, int y, int w, int h);
    void DrawNumbers(int x, int y, std::size_t dirtyBeforeDraw) const;
    void DrawHint(int x, int y, int scale) const;

    // World units -> the field box. ONE mapping, used by the boxes AND the lines, so
    // a line cannot disagree with the box it connects.
    int FieldX(float worldX, int x, int w) const;
    int FieldY(float worldY, int y, int h) const;

    SDL_Renderer  *renderer_;
    int            windowWidth_, windowHeight_;
    bool           isDebugging_;
    AssetStore_Ptr assetStore_;
    Logger         logger_;
    bool          &isRunning_;

    int millisecondsPreviousFrame_ = 0;

    dirty::TransformTree tree_;
    int         root_ = -1;
    int         branch_ = -1;         // the one branch the C key moves
    float       phase_ = 0.0f;
    bool        autoMove_ = true;     // dirst all 17 nodes every frame while on
    std::string status_ = "AUTO MOVE ON";
    std::size_t lastDirty_ = 0;       // what was stale when the frame began
};
