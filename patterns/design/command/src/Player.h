#pragma once

#include <glm/glm.hpp>

#include "Direction.h"

// The actor a command acts upon. Holds a position and moves a fixed step per
// command, clamped to the play area.
class Player {
public:
  Player(glm::vec2 pos, int boundsW, int boundsH);

  void move(Direction direction);

  glm::vec2 position() const { return m_pos; }
  int       size() const     { return m_size; }

private:
  glm::vec2 m_pos;
  int       m_boundsW;
  int       m_boundsH;
  int       m_size = 40;
  float     m_step = 8.f;
};
