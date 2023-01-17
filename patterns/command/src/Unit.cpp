#include <iostream>

#include "Unit.h"

Unit::Unit(int x, int y) : x_(x), y_(y) {}

int Unit::x() const { return x_; }
int Unit::y() const { return y_; }

void Unit::moveTo(int x, int y) {
  x_ = x;
  y_ = y;
  std::cout << "Unit moved to (" << x_ << ", " << y_ << ")"
            << "\n";
}