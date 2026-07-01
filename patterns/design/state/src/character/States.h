#pragma once

#include "CharacterState.h"

// The concrete states. They hold no per-character data, so a single shared
// ("static") instance of each is enough — see the book's "Static States".

class StandingState : public CharacterState {
public:
  static StandingState &instance();
  CharacterState *handleInput(Character &, Input) override;
  const char *name() const override { return "Standing"; }
};

class JumpingState : public CharacterState {
public:
  static JumpingState &instance();
  CharacterState *handleInput(Character &, Input) override;
  CharacterState *update(Character &, float dt) override; // lands over time
  const char *name() const override { return "Jumping"; }
};

class DuckingState : public CharacterState {
public:
  static DuckingState &instance();
  CharacterState *handleInput(Character &, Input) override;
  const char *name() const override { return "Ducking"; }
};
