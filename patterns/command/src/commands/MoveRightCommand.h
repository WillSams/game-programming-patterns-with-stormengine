#pragma once

#include "../Sprite.h"
#include "MoveCommand.h"

class MoveRightCommand : public MoveCommand {
public:
  Direction getDirection() override;
};