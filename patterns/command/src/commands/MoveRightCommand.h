#pragma once

#include "Command.h"

class MoveRightCommand : public Command {
public:
  Direction getDirection() override;
};
