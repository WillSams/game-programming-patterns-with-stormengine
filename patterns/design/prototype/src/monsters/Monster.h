#pragma once

#include <memory>
#include <string>

struct Color {
  int r, g, b;
};

// The prototype's interface: every monster knows how to clone itself, so a
// spawner can produce more of the same without knowing the concrete type.
// SDL-free so the cloning logic is testable on its own.
class Monster {
public:
  Monster(int health, float speed, Color color)
      : m_health(health), m_speed(speed), m_color(color) {}
  virtual ~Monster() = default;

  virtual std::unique_ptr<Monster> clone() const = 0;
  virtual std::string name() const = 0;

  int   health() const { return m_health; }
  float speed() const  { return m_speed; }
  Color color() const  { return m_color; }

protected:
  int   m_health;
  float m_speed;
  Color m_color;
};
