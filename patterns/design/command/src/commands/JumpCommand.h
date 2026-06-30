#pragma once

#include "Command.h"

class JumpCommand : public Command {
public:
  Direction getDirection() override;
};
