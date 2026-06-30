#pragma once

#include "Command.h"

class MoveLeftCommand : public Command {
public:
  Direction getDirection() override;
};
