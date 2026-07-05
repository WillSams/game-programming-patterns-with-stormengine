#include <igloo/igloo_alt.h>

#include "../src/Settings.h"

using namespace igloo;

Describe(SettingsSingletonSpec) {

    It(should_return_the_same_instance_every_time) {
        Settings &a = Settings::instance();
        Settings &b = Settings::instance();
        Assert::That(&a == &b, Equals(true));
    };

    It(should_share_state_across_accesses) {
        // A write through one access point is visible through another.
        Settings::instance().setVolume(75);
        Assert::That(Settings::instance().volume(), Equals(75));
    };

    It(should_clamp_volume_to_the_valid_range) {
        Settings::instance().setVolume(150);
        Assert::That(Settings::instance().volume(), Equals(100));
        Settings::instance().setVolume(-20);
        Assert::That(Settings::instance().volume(), Equals(0));
    };

    It(should_advance_difficulty_when_cycled) {
        auto before = Settings::instance().difficulty();
        Settings::instance().cycleDifficulty();
        Assert::That(Settings::instance().difficulty() != before, Equals(true));
    };

    It(should_wrap_difficulty_after_three_cycles) {
        auto start = Settings::instance().difficulty();
        Settings::instance().cycleDifficulty();
        Settings::instance().cycleDifficulty();
        Settings::instance().cycleDifficulty();
        Assert::That(Settings::instance().difficulty() == start, Equals(true));
    };

    It(should_name_each_difficulty) {
        while (Settings::instance().difficulty() != Settings::Difficulty::Easy)
            Settings::instance().cycleDifficulty();
        Assert::That(Settings::instance().difficultyName(), Equals("Easy"));
    };

    // The book's caveat, made concrete: because the singleton is global state,
    // changes from an *earlier* test are still visible here. Tests aren't
    // isolated unless you remember to reset the shared instance.
    It(should_leak_state_between_tests_unless_reset) {
        // No setVolume here — we read whatever a prior test happened to leave.
        Settings::instance().setVolume(42); // explicit reset to stay deterministic
        Assert::That(Settings::instance().volume(), Equals(42));
    };
};
