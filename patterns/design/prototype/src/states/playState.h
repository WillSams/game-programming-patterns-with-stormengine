#pragma once

#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <random>
#include <string>
#include <vector>

#include <stormengine2/assetStore.h>
#include <stormengine2/logger.h>
#include <stormengine2/states/gameState.h>

#include "../monsters/Spawner.h"

// Demonstrates the Prototype pattern: a Spawner clones a prototype monster.
// Keys 1/2/3 swap the prototype; Space spawns a clone at a random spot. Every
// spawned monster is an independent copy of the current prototype.
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

    void spawnOne();

    SDL_Renderer  *renderer_;
    int            windowWidth_, windowHeight_;
    bool           isDebugging_;
    AssetStore_Ptr assetStore_;
    Logger         logger_;
    bool          &isRunning_;

    struct ActiveMonster {
        std::unique_ptr<Monster> monster;
        glm::vec2 pos;
        glm::vec2 vel;
    };

    Spawner                    spawner_;
    std::vector<ActiveMonster> monsters_;
    std::mt19937               rng_;

    int millisecondsPreviousFrame_ = 0;
};
