#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <vector>

#include <stormengine2/assetStore.h>
#include <stormengine2/logger.h>
#include <stormengine2/states/gameState.h>

#include "../services/serviceLocator.h"

// ── The on-screen provider ───────────────────────────────────────────────────
//
// The one registered while the demo runs: it appends what it was told, and the
// screen draws that list. So "what the game called" and "what the service heard"
// are two different lists produced by different objects, and the whole point of the
// pattern is visible as a GAP between them: register the null service and the game
// keeps calling while this list stops growing.
//
// ⚠️ Deliberately NOT shared with the specs' RecordingAudioService. A test double
// belongs with the test it serves -- a spy whose job is to be asserted against --
// while this one is display state for one screen. Two recorders that look alike but
// answer different questions is not the second appearance of one decision.
class OnScreenAudioService : public services::AudioService {
public:
    void PlaySound(const std::string &tag) override { log_.push_back("HEARD SOUND " + tag); }
    void PlayMusic(const std::string &tag) override { log_.push_back("HEARD MUSIC " + tag); }
    void StopAll() override { log_.push_back("HEARD STOP"); }
    const char *Name() const override { return "CONSOLE"; }

    const std::vector<std::string> &Log() const { return log_; }
    void Clear() { log_.clear(); }

private:
    std::vector<std::string> log_;
};

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

    // The GAME's part: it names no implementation at all. This is the code the
    // pattern exists to keep clean, and it is the only code that calls audio.
    void CallSound(const std::string &tag);
    void CallMusic(const std::string &tag);
    void CallStopAll();

    void DrawCalled(int x, int y) const;
    void DrawRegistered(int x, int y) const;
    void DrawHeard(int x, int y) const;
    void DrawHint(int x, int y, int scale) const;

    SDL_Renderer  *renderer_;
    int            windowWidth_, windowHeight_;
    bool           isDebugging_;
    AssetStore_Ptr assetStore_;
    Logger         logger_;
    bool          &isRunning_;

    int millisecondsPreviousFrame_ = 0;

    OnScreenAudioService consoleAudio_;
    std::vector<std::string> called_;   // the game's own log
    // ⚠️ THERE IS NO `heard_` COUNTER. There was one, and it was a second record of
    // `consoleAudio_.Log().size()` -- two records of one fact can disagree, and a
    // stale one indexes that log out of range. It is derived where it is drawn.
    std::string status_ = "CONSOLE AUDIO REGISTERED";
};
