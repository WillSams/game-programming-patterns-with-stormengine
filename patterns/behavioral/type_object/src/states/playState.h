#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <vector>

#include <stormengine2/assetStore.h>
#include <stormengine2/logger.h>
#include <stormengine2/states/gameState.h>

#include "../types/bestiary.h"
#include "../types/monster.h"

// The Type Object demo: every kind of creature is one class, and what varies
// between kinds is an object. The pattern lives in src/types/ (pure, spec'd); this
// screen is presentation -- the breeds, the monsters spawned from them, and the
// fact that many monsters point at ONE breed.
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

    void Spawn();
    void DamageAll(int amount);
    void HealAll(int amount);
    void DrawBreeds(int x, int y, int scale) const;
    void DrawSpawned(int x, int y, int scale) const;
    void DrawSharing(int x, int y, int scale) const;
    void DrawHint(int x, int y, int scale) const;

    SDL_Renderer  *renderer_;
    int            windowWidth_, windowHeight_;
    bool           isDebugging_;
    AssetStore_Ptr assetStore_;
    Logger         logger_;
    bool          &isRunning_;

    int millisecondsPreviousFrame_ = 0;

    // Pointers to the static breeds: a Monster holds a reference to its type
    // object, so whatever this points at must outlive the monsters (bestiary.h).
    std::vector<const types::Breed *> breeds_;
    std::vector<types::Monster>       spawned_;
    std::size_t index_ = 0;
    int         nextId_ = 1;
    std::string status_ = "READY";
};
