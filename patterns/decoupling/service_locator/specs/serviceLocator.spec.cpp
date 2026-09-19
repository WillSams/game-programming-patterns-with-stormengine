#include <igloo/igloo_alt.h>

#include <string>
#include <vector>

#include "../src/services/serviceLocator.h"

using namespace igloo;
using namespace services;

// A test double for the service. It records what it was told, which is the
// pattern's whole payoff for testing: the CALLER can be exercised with no device,
// no window and no sound, and the spec can ask what the service actually saw.
class RecordingAudioService : public AudioService {
public:
    explicit RecordingAudioService(std::string name) : name_(std::move(name)) {}

    void PlaySound(const std::string &tag) override { log_.push_back("SOUND " + tag); }
    void PlayMusic(const std::string &tag) override { log_.push_back("MUSIC " + tag); }
    void StopAll() override { log_.push_back("STOP"); }
    const char *Name() const override { return name_.c_str(); }

    const std::vector<std::string> &Log() const { return log_; }
    void Clear() { log_.clear(); }
    bool Saw(const std::string &line) const {
        for (const auto &l : log_) if (l == line) return true;
        return false;
    }

private:
    std::string              name_;
    std::vector<std::string> log_;
};

// ── THE CODE UNDER TEST IS THE CALLER, AND IT NAMES NO PROVIDER ──────────────
//
// This is the pattern's claim in one function: it reaches the audio service
// through the locator and nowhere mentions NullAudioService, RecordingAudioService
// or any other implementation. The specs below run it against two different
// providers and against nothing at all.
static void LevelStart() {
    ServiceLocator::Audio().PlaySound("whistle");
    ServiceLocator::Audio().PlayMusic("arena-loop");
}

// A caller that keeps its OWN log, so a spec can separate "what the caller did"
// from "what the service received" -- which is exactly the seam the pattern adds.
static std::vector<std::string> &CallerLog() {
    static std::vector<std::string> log;
    return log;
}
static void CallerPlays(const std::string &tag) {
    CallerLog().push_back("CALLER " + tag);
    ServiceLocator::Audio().PlaySound(tag);
}

// ⚠️ THE PROVIDERS ARE AT FILE SCOPE, NOT MEMBERS OF THE Describe. igloo's nested
// Describe blocks are nested CLASSES, so a case inside one cannot see the enclosing
// describe's data members -- and a provider the cases cannot name is no use. They
// keep their own lifetime here rather than being owned by the locator, which is
// itself one of the things under test.
static RecordingAudioService alpha{"ALPHA"};
static RecordingAudioService beta{"BETA"};
static RecordingAudioService callerReceiver{"CALLER-RX"};

// ⚠️ THE LOCATOR IS A GLOBAL, SO THE FIXTURE PUTS IT BACK. The chapter warns that a
// locator makes every test share one fixture -- this is that warning implemented
// rather than quoted, and it is why Provide(nullptr) exists.
Describe(ServiceLocatorSpec) {

  void SetUp() override {
      ServiceLocator::Provide(nullptr);
      CallerLog().clear();
      alpha.Clear();
      beta.Clear();
      callerReceiver.Clear();
  }
  void TearDown() override {
      ServiceLocator::Provide(nullptr);
      CallerLog().clear();
  }

  // ── 1. THE CONTRACT THAT MAKES CALLERS SIMPLE ──────────────────────────────
  Describe(NeverReturnsNull) {
    It(returns_the_null_service_when_nothing_was_provided) {
      Assert::That(ServiceLocator::HasProvider(), IsFalse());
      Assert::That(&ServiceLocator::Audio() == &NullAudio(), IsTrue());
      Assert::That(ServiceLocator::CurrentName(), Equals("NULL"));
    };

    It(returns_the_null_service_after_being_handed_nullptr) {
      ServiceLocator::Provide(&alpha);
      ServiceLocator::Provide(nullptr);                 // the book's own idiom
      Assert::That(&ServiceLocator::Audio() == &NullAudio(), IsTrue());
      Assert::That(ServiceLocator::HasProvider(), IsFalse());
    };

    // The null service is not a sentinel to be checked -- it is a service, and
    // calling it is legal. This is what replaces `if (audio)` at every call site.
    It(absorbs_every_call_without_crashing) {
      ServiceLocator::Audio().PlaySound("nobody-is-listening");
      ServiceLocator::Audio().PlayMusic("nothing");
      ServiceLocator::Audio().StopAll();
      Assert::That(ServiceLocator::HasProvider(), IsFalse());
    };
  };

  // ── 2. THE PROVIDER IS REPLACED WHILE THE GAME RUNS ────────────────────────
  // The chapter's motivating case: the player mutes, and the game keeps calling.
  Describe(ProvidersCanBeSwappedAtRuntime) {
    It(calls_follow_the_swapped_provider) {
      ServiceLocator::Provide(&alpha);
      LevelStart();
      Assert::That(alpha.Saw("SOUND whistle"), IsTrue());
      Assert::That(alpha.Saw("MUSIC arena-loop"), IsTrue());

      ServiceLocator::Provide(&beta);
      LevelStart();
      Assert::That(beta.Saw("SOUND whistle"), IsTrue());
    };

    It(stops_calling_the_previous_provider_after_a_swap) {
      ServiceLocator::Provide(&alpha);
      LevelStart();
      const std::size_t before = alpha.Log().size();

      ServiceLocator::Provide(&beta);
      LevelStart();
      Assert::That(alpha.Log().size(), Equals(before));   // alpha heard no more
    };

    // The payoff in one case: the same caller, two providers, and the caller's own
    // behaviour is IDENTICAL. It did not choose, and it did not change.
    It(leaves_the_callers_own_behaviour_identical_across_providers) {
      ServiceLocator::Provide(&callerReceiver);
      CallerPlays("goal");
      CallerPlays("horn");
      const std::vector<std::string> withProvider = CallerLog();

      ServiceLocator::Provide(nullptr);
      CallerLog().clear();
      CallerPlays("goal");
      CallerPlays("horn");

      Assert::That(CallerLog(), Equals(withProvider));
      Assert::That(callerReceiver.Saw("SOUND goal"), IsTrue());  // and it was heard
    };
  };

  // ── 3. THE LOCATOR DOES NOT OWN WHAT IT IS HANDED ──────────────────────────
  // A locator that destroyed its provider on reset would be a lifetime trap the
  // caller never signed up for. Pinned because "who owns it" is exactly the
  // question a global access point makes ambiguous.
  Describe(DoesNotOwnTheProvider) {
    It(leaves_the_provider_alive_and_usable_after_a_reset) {
      ServiceLocator::Provide(&alpha);
      LevelStart();
      ServiceLocator::Provide(nullptr);            // unregister

      // alpha is still here -- resetting the locator destroyed nothing.
      alpha.PlaySound("still-mine");
      Assert::That(alpha.Saw("SOUND still-mine"), IsTrue());
      Assert::That(ServiceLocator::HasProvider(), IsFalse());
    };
  };

  // ── 4. THE GLOBAL'S OWN HAZARD, FROM THE OTHER SIDE ────────────────────────
  // State provably persists between cases unless it is reset, which is the cost of
  // a global access point and the reason the fixture above exists.
  Describe(StateLeaksUnlessReset) {
    It(keeps_the_provider_until_something_puts_it_back) {
      ServiceLocator::Provide(&alpha);
      Assert::That(ServiceLocator::CurrentName(), Equals("ALPHA"));
      ServiceLocator::Provide(nullptr);
      Assert::That(ServiceLocator::CurrentName(), Equals("NULL"));
    };
  };
};
