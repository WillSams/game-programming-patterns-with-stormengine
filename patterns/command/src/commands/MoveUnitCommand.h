#include "../Unit.h"
#include "Command.h"

class MoveUnitCommand : public Command {
public:
  MoveUnitCommand(Unit &unit, int x, int y) : unit_(unit), x_(x), y_(y) {
    x_before_ = unit.x();
    y_before_ = unit.y();
  }

  void execute() override { unit_.moveTo(x_, y_); }

  void undo() override { unit_.moveTo(x_before_, y_before_); }

private:
  Unit &unit_;
  int x_, y_;
  int x_before_, y_before_;
};