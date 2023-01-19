#pragma once

#include "../Sprite.h"
#include "MoveCommand.h"

class MoveLeftCommand : public MoveCommand {
public:
  Direction getDirection() override;
};