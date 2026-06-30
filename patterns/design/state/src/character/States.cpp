#include "States.h"

#include "Character.h"

StandingState &StandingState::instance() { static StandingState s; return s; }
JumpingState  &JumpingState::instance()  { static JumpingState s;  return s; }
DuckingState  &DuckingState::instance()  { static DuckingState s;  return s; }

CharacterState *StandingState::handleInput(Character &, Input input) {
  switch (input) {
  case Input::Jump: return &JumpingState::instance();
  case Input::Duck: return &DuckingState::instance();
  default:          return this;
  }
}

CharacterState *JumpingState::handleInput(Character &, Input input) {
  // Pressing down mid-air dives into a duck (resolved on landing).
  if (input == Input::Duck) return &DuckingState::instance();
  return this;
}

CharacterState *JumpingState::update(Character &character, float dt) {
  character.addAirTime(dt);
  if (character.airTime() >= Character::JUMP_DURATION)
    return &StandingState::instance(); // land
  return this;
}

CharacterState *DuckingState::handleInput(Character &, Input input) {
  if (input == Input::Stand) return &StandingState::instance();
  return this;
}
