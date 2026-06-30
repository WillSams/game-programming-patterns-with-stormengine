#include "Player.h"

#include <algorithm>

Player::Player(glm::vec2 pos, int boundsW, int boundsH)
    : m_pos(pos), m_boundsW(boundsW), m_boundsH(boundsH) {}

void Player::move(Direction direction) {
  switch (direction) {
  case LEFT:  m_pos.x -= m_step; break;
  case RIGHT: m_pos.x += m_step; break;
  case UP:    m_pos.y -= m_step; break;
  case DOWN:  m_pos.y += m_step; break;
  }

  m_pos.x = std::max(0.f, std::min(m_pos.x, static_cast<float>(m_boundsW - m_size)));
  m_pos.y = std::max(0.f, std::min(m_pos.y, static_cast<float>(m_boundsH - m_size)));
}
