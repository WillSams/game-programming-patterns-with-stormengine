#include "Command.h"

#include "../Sprite.h"

class MoveCommand : public Command {
public:
  virtual Direction getDirection() = 0;
};
