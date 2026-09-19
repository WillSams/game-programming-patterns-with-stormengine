#pragma once

#include <SDL2/SDL.h>
#include <string>

#include <stormengine2/assetStore.h>
#include <stormengine2/logger.h>
#include <stormengine2/states/gameState.h>

#include "../vm/bytecodeVm.h"
#include "../vm/spells.h"

// The Bytecode demo: a spell is DATA, and this screen lets you watch the machine
// run one. The pattern itself lives in src/vm/ (pure, spec'd); everything here is
// presentation -- a listing with the program counter on it, the machine's state,
// and its stack.
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

    // A spell and the name the demo shows for it, together, so the list the
    // number keys index and the list the screen prints cannot disagree.
    struct Demo {
        const char       *name;
        bytecode::Program program;
    };
    static const std::vector<Demo> &Spells();
    void SelectSpell(std::size_t index);
    void DrawProgram(int x, int y, int scale) const;
    void DrawState(int x, int y, int scale) const;
    void DrawStack(int x, int y, int scale) const;
    void DrawSpells(int x, int y, int scale) const;
    void DrawHint(int x, int y, int scale) const;

    SDL_Renderer  *renderer_;
    int            windowWidth_, windowHeight_;
    bool           isDebugging_;
    AssetStore_Ptr assetStore_;
    Logger         logger_;
    bool          &isRunning_;

    int millisecondsPreviousFrame_ = 0;

    bytecode::BytecodeVm vm_;
    std::size_t          spellIndex_ = 0;
    std::string          status_ = "READY";
};
