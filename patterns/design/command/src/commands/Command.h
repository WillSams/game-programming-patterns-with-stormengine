#pragma once

#include "../Direction.h"

// Base of the Command pattern: a request encapsulated as an object. Concrete
// commands resolve to the action they represent.
class Command {
public:
  virtual ~Command() = default;
  virtual Direction getDirection() = 0;
};
