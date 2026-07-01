#pragma once

#include "Monster.h"

class Ghost : public Monster {
public:
  Ghost() : Monster(15, 90.f, {235, 235, 235}) {} // white

  std::unique_ptr<Monster> clone() const override {
    return std::make_unique<Ghost>(*this); // copy-construct a duplicate
  }
  std::string name() const override { return "Ghost"; }
};
