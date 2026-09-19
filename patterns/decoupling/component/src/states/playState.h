#pragma once

#include <SDL2/SDL.h>
#include <memory>
#include <string>
#include <vector>

#include <stormengine2/assetStore.h>
#include <stormengine2/logger.h>
#include <stormengine2/states/gameState.h>

#include "../components/components.h"

// The Component demo: three kinds of thing, one class, and what each one does is
// what it is made of. The pattern lives in src/components/ (pure, spec'd); this
// screen is presentation -- the composition of each entity and a box that draws
// the draw items they produce.
//
// ⚠️ EACH KEY ADVANCES ONE TICK, deliberately. The components are what is being
// demonstrated, and stepping them makes the composition visible: press RIGHT and
// the input component sets a velocity, the physics component moves the entity, and
// the appearance component describes it where it now is.
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
    void Tick(const components::InputState &input, int times = 1);
    void DrawEntities(int x, int y, int scale) const;
    void DrawInspect(int x, int y, int scale) const;
    void DrawWorld(int x, int y, int scale) const;
    void DrawHint(int x, int y, int scale) const;

    SDL_Renderer  *renderer_;
    int            windowWidth_, windowHeight_;
    bool           isDebugging_;
    AssetStore_Ptr assetStore_;
    Logger         logger_;
    bool          &isRunning_;

    int millisecondsPreviousFrame_ = 0;

    std::vector<std::unique_ptr<components::Entity>> entities_;
    std::size_t index_ = 0;
    int         ticks_ = 0;
    std::string status_ = "READY";
};
