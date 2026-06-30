#pragma once

#include <memory>

#include "Monster.h"

// Holds a single prototype monster and produces new ones by cloning it.
// Swapping the prototype changes what gets spawned — no per-type spawner class.
class Spawner {
public:
  explicit Spawner(std::unique_ptr<Monster> prototype)
      : m_prototype(std::move(prototype)) {}

  std::unique_ptr<Monster> spawn() const { return m_prototype->clone(); }

  void setPrototype(std::unique_ptr<Monster> prototype) {
    m_prototype = std::move(prototype);
  }
  const Monster &prototype() const { return *m_prototype; }

private:
  std::unique_ptr<Monster> m_prototype;
};
