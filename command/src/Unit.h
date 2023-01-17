#pragma once

#include <iostream>

class Unit {

public:
  Unit(int x, int y);

  int x() const;
  int y() const;

  void moveTo(int x, int y);

private:
  int x_, y_;
};