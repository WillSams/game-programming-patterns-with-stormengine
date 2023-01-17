#include <gtest/gtest.h>

#include "../src/commands/MoveUnitCommand.h"

TEST(MoveUnitCommand, execute) {
  Unit unit(0, 0);
  MoveUnitCommand command(unit, 1, 1);
  command.execute();
  ASSERT_EQ(1, unit.x());
  ASSERT_EQ(1, unit.y());
}

TEST(MoveUnitCommand, undo) {
  Unit unit(0, 0);
  MoveUnitCommand command(unit, 1, 1);
  command.execute();
  command.undo();
  ASSERT_EQ(0, unit.x());
  ASSERT_EQ(0, unit.y());
}