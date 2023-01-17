#include <gtest/gtest.h>

#include "../src/Unit.h"

TEST(Unit, constructor) {
  Unit unit(0, 0);
  ASSERT_EQ(0, unit.x());
  ASSERT_EQ(0, unit.y());
}

TEST(Unit, moveTo) {
  Unit unit(0, 0);
  unit.moveTo(1, 1);
  ASSERT_EQ(1, unit.x());
  ASSERT_EQ(1, unit.y());
}