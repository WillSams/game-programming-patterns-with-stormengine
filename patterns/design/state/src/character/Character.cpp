#include "Character.h"

#include "States.h"

Character::Character() : m_state(&StandingState::instance()) {}

void Character::handleInput(Input input) {
  changeState(m_state->handleInput(*this, input));
}

void Character::update(float dt) {
  changeState(m_state->update(*this, dt));
}

const char *Character::stateName() const { return m_state->name(); }

void Character::changeState(CharacterState *next) {
  if (next != m_state) {
    m_state   = next;
    m_airTime = 0.f; // fresh state, fresh timer
  }
}
