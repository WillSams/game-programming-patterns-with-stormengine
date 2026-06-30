#include "ScoreSubject.h"

#include <algorithm>

void ScoreSubject::addObserver(ScoreObserver *observer) {
  m_observers.push_back(observer);
}

void ScoreSubject::removeObserver(ScoreObserver *observer) {
  m_observers.erase(
      std::remove(m_observers.begin(), m_observers.end(), observer),
      m_observers.end());
}

void ScoreSubject::addPoints(int points) {
  m_score += points;
  notify();
}

void ScoreSubject::notify() {
  for (ScoreObserver *observer : m_observers)
    observer->onScoreChanged(m_score);
}
