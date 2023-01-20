#pragma once

#include "../Sprite.h"
#include "MoveCommand.h"

class JumpCommand : public MoveCommand {
public:
  Direction getDirection() override;
};