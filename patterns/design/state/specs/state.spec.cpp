#include <igloo/igloo_alt.h>
#include <string>

#include "../src/character/Character.h"

using namespace igloo;

Describe(CharacterStateMachineSpec) {

    It(starts_standing) {
        Character c;
        Assert::That(std::string(c.stateName()), Equals("Standing"));
    };

    It(jumps_from_standing) {
        Character c;
        c.handleInput(Input::Jump);
        Assert::That(std::string(c.stateName()), Equals("Jumping"));
    };

    It(ducks_from_standing) {
        Character c;
        c.handleInput(Input::Duck);
        Assert::That(std::string(c.stateName()), Equals("Ducking"));
    };

    It(stands_up_from_ducking) {
        Character c;
        c.handleInput(Input::Duck);
        c.handleInput(Input::Stand);
        Assert::That(std::string(c.stateName()), Equals("Standing"));
    };

    It(dives_to_a_duck_from_a_jump) {
        Character c;
        c.handleInput(Input::Jump);
        c.handleInput(Input::Duck);
        Assert::That(std::string(c.stateName()), Equals("Ducking"));
    };

    It(ignores_input_a_state_does_not_handle) {
        Character c; // Standing
        c.handleInput(Input::Stand); // no-op
        Assert::That(std::string(c.stateName()), Equals("Standing"));
    };

    It(lands_after_the_jump_duration) {
        Character c;
        c.handleInput(Input::Jump);
        c.update(Character::JUMP_DURATION + 0.1f);
        Assert::That(std::string(c.stateName()), Equals("Standing"));
    };

    It(stays_airborne_before_landing) {
        Character c;
        c.handleInput(Input::Jump);
        c.update(0.1f);
        Assert::That(std::string(c.stateName()), Equals("Jumping"));
    };

    It(resets_air_time_on_each_jump) {
        Character c;
        c.handleInput(Input::Jump);
        c.update(0.3f);
        c.update(0.3f);                 // total 0.6 > duration -> lands
        c.handleInput(Input::Jump);     // jump again; air time should reset
        c.update(0.1f);
        Assert::That(std::string(c.stateName()), Equals("Jumping"));
    };
};
