#pragma once

#include "commands/Command.h"

#include "Sprite.h"
class Player {
public:
  Player(Sprite sprite);
  ~Player() = default;

  void render();
  void update(Command *command);

private:
  Sprite m_sprite;
};
