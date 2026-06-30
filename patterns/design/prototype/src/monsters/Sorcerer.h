#pragma once

#include "Monster.h"

class Sorcerer : public Monster {
public:
  Sorcerer() : Monster(20, 110.f, {210, 70, 70}) {} // red

  std::unique_ptr<Monster> clone() const override {
    return std::make_unique<Sorcerer>(*this);
  }
  std::string name() const override { return "Sorcerer"; }
};
