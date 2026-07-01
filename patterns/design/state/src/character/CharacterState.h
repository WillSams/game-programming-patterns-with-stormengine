#pragma once

class Character;

// The inputs the state machine reacts to.
enum class Input { Jump, Duck, Stand };

// A state in the character's finite state machine. Each state decides how to
// react to input and how to advance over time, returning the state to be in
// next (return `this` for no change). This replaces a thicket of boolean flags
// and if-statements with one object per state.
class CharacterState {
public:
  virtual ~CharacterState() = default;

  virtual CharacterState *handleInput(Character &character, Input input) = 0;
  virtual CharacterState *update(Character &character, float dt) { return this; }
  virtual const char *name() const = 0;
};
