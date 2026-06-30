#pragma once

#include <string>

// A textbook Singleton: exactly one instance, reachable from anywhere via
// Settings::instance(). Copying is disabled so no second instance can exist.
//
// (The book is skeptical of this pattern — see the README. The point of the
// example is to show it honestly, caveats and all.)
class Settings {
public:
  // The single global access point. Uses a function-local static ("Meyers
  // singleton"): created once, lazily, and thread-safe since C++11.
  static Settings &instance();

  Settings(const Settings &)            = delete;
  Settings &operator=(const Settings &) = delete;

  int  volume() const { return m_volume; }
  void setVolume(int v); // clamped to [0, 100]

  enum class Difficulty { Easy, Normal, Hard };
  Difficulty  difficulty() const { return m_difficulty; }
  void        cycleDifficulty();
  std::string difficultyName() const;

private:
  Settings() = default; // only instance() may construct it

  int        m_volume     = 50;
  Difficulty m_difficulty = Difficulty::Normal;
};
