#pragma once

#include "CharacterState.h"

// The context: delegates input and updates to its current state, and switches
// state whenever a state hands back a different one. Transient data the states
// need (here, time spent airborne) lives on the character, keeping the states
// themselves data-free.
class Character {
public:
  static constexpr float JUMP_DURATION = 0.5f; // seconds aloft before landing

  Character();

  void handleInput(Input input);
  void update(float dt);

  const char *stateName() const;

  float airTime() const { return m_airTime; }
  void  addAirTime(float dt) { m_airTime += dt; }

private:
  void changeState(CharacterState *next);

  CharacterState *m_state;
  float           m_airTime = 0.f;
};
