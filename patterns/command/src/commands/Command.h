#pragma once

#include "../Sprite.h"

class Command {
public:
  virtual Direction getDirection() = 0;
};