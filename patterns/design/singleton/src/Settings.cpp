#include "Settings.h"

#include <algorithm>

Settings &Settings::instance() {
  static Settings s; // constructed on first use, destroyed at program exit
  return s;
}

void Settings::setVolume(int v) {
  m_volume = std::clamp(v, 0, 100);
}

void Settings::cycleDifficulty() {
  m_difficulty =
      static_cast<Difficulty>((static_cast<int>(m_difficulty) + 1) % 3);
}

std::string Settings::difficultyName() const {
  switch (m_difficulty) {
  case Difficulty::Easy:   return "Easy";
  case Difficulty::Normal: return "Normal";
  case Difficulty::Hard:   return "Hard";
  }
  return "?";
}
