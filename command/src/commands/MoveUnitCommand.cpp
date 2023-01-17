#include "MoveUnitCommand.h"

#include "../Unit.h"

MoveUnitCommand::MoveUnitCommand(Unit &unit, int x, int y)
    : unit_(unit), x_(x), y_(y) {
  x_before_ = unit.x();
  y_before_ = unit.y();
};

void MoveUnitCommand::execute() { unit_.moveTo(x_, y_); };
void MoveUnitCommand::undo() { unit_.moveTo(x_before_, y_before_); };