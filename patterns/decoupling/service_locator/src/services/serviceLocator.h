#pragma once

#include "service.h"

// ── Service Locator pattern, part 2 of 3: THE LOCATOR ────────────────────────
//
// The point of the pattern: the game asks for the service it needs WITHOUT naming
// the class that provides it, and without the provider being threaded through
// every constructor between here and the code that actually wants it.
//
// ── ⚠️ THE CHAPTER IS EXPLICITLY UNSURE ABOUT THIS PATTERN, AND SO IS THIS ──
//
// It is "one of the most misunderstood" of the book's patterns: the locator is a
// GLOBAL, so it hides dependencies rather than declaring them -- a class that calls
// ServiceLocator::Audio() looks like it needs nothing, and the only way to learn
// otherwise is to read it. The chapter recommends DEPENDENCY INJECTION where the
// choice is available, and this repo takes that seriously elsewhere: the demos here
// pass their dependencies as constructor arguments.
//
// So the honest reading of this pattern is: it is what you reach for when a global
// really is what you have (a platform service, an audio device) and threading it
// through forty constructors would be worse -- and even then, the null service and
// a way to UNREGISTER are what keep it from becoming a hazard. Both are pinned by
// spec below rather than left as advice.
//
// THE CONTRACT THIS IMPLEMENTATION KEEPS:
//
//   1. Audio() NEVER RETURNS NULL. With nothing provided it returns the null
//      service, so no caller anywhere writes a null check.
//   2. Provide(nullptr) UNREGISTERS. That is the book's own idiom, and it is the
//      path that makes a test able to put the world back.
//   3. THE LOCATOR DOES NOT OWN THE PROVIDER. It stores a reference; the caller
//      keeps the provider alive. Resetting the locator does NOT destroy anything --
//      pinned by a spec, because a locator that deleted what it was handed would be
//      a lifetime trap nobody signed up for.
//
// TWO LIMITS, STATED RATHER THAN LEFT TO BE DISCOVERED:
//   * A swap is not synchronised, so providing while another thread is mid-call is
//     a data race. The book's version has the same one; this is a single-threaded
//     seam, and making it atomic would be a lock on a read path that does not need
//     it. Say so if that changes.
//   * Nothing here is safe to call after `main` returns: a provider at file scope
//     is destroyed with the statics, and the null service with them. Nothing calls
//     it then, and a game has no reason to.
namespace services {

class ServiceLocator {
public:
    // The access point. Cannot fail, cannot be null.
    static AudioService &Audio() { return *instance().current_; }

    // Hand it a provider, or hand it nullptr to put the null service back. The
    // locator keeps a reference and does not take ownership.
    static void Provide(AudioService *service) {
        instance().current_ = service ? service : &NullAudio();
    }

    // Whether a real provider -- rather than the null service -- is installed.
    static bool HasProvider() { return instance().current_ != &NullAudio(); }

    // The current provider's name, for the screen.
    static const char *CurrentName() { return instance().current_->Name(); }

private:
    static ServiceLocator &instance() {
        static ServiceLocator locator;
        return locator;
    }

    // ⚠️ A GLOBAL, WHICH IS WHY THE SPECS RESET IT. The chapter warns that a locator
    // makes every test share one fixture; the `Reset` case below is the same trap
    // seen from the other side, and the fixture resets on entry for that reason.
    AudioService *current_ = &NullAudio();
};

} // namespace services
