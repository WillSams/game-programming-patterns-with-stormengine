#pragma once

#include "../Sprite.h"

class Command {
public:
  ~Command() = default;
  virtual Direction getDirection() = 0;
};