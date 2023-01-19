#pragma once

#include "Command.h"

class JumpCommand : public Command {
public:
  void execute() override;
};