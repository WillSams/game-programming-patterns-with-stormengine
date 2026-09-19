#pragma once

#include <string>

// ── Service Locator pattern, part 1 of 3: THE SERVICE ────────────────────────
//
// The interface the rest of the game talks to. The game never names an audio
// implementation -- only this.
namespace services {

class AudioService {
public:
    virtual ~AudioService() = default;

    virtual void PlaySound(const std::string &tag) = 0;
    virtual void PlayMusic(const std::string &tag) = 0;
    virtual void StopAll() = 0;

    // For the screen, and for a readable spec failure.
    virtual const char *Name() const = 0;
};

// ── THE NULL SERVICE. This is the chapter's own recommendation and the reason a
// caller never writes `if (audio)`. Nothing here has to work: every call is a
// no-op, and the game keeps running, which is what should happen when a sound
// cannot be played.
//
// ⚠️ Reached through an accessor rather than a namespace-scope object: a global
// constructed at namespace scope can be used by another global's constructor
// before it exists (the static-initialisation-order fiasco), and a function-local
// static has no such window.
class NullAudioService : public AudioService {
public:
    void PlaySound(const std::string &) override {}
    void PlayMusic(const std::string &) override {}
    void StopAll() override {}
    const char *Name() const override { return "NULL"; }
};

inline AudioService &NullAudio() {
    static NullAudioService instance;
    return instance;
}

} // namespace services
