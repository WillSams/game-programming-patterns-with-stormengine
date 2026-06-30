#pragma once

#include "Monster.h"

class Demon : public Monster {
public:
  Demon() : Monster(40, 60.f, {150, 90, 200}) {} // purple

  std::unique_ptr<Monster> clone() const override {
    return std::make_unique<Demon>(*this);
  }
  std::string name() const override { return "Demon"; }
};
