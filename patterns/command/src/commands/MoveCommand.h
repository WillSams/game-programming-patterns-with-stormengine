#pragma once

#include "../Sprite.h"
#include "Command.h"

class MoveCommand : public Command {
public:
  virtual Direction getDirection() = 0;
};