#pragma once

#include <vector>

#include "ScoreObserver.h"

// Subject (observable): owns the score and notifies its observers whenever it
// changes. It has no idea what the observers do with the news — that's the
// decoupling the Observer pattern buys you.
class ScoreSubject {
public:
  void addObserver(ScoreObserver *observer);
  void removeObserver(ScoreObserver *observer);

  void addPoints(int points); // mutates score, then notifies
  int  score() const { return m_score; }

  int observerCount() const { return static_cast<int>(m_observers.size()); }

private:
  void notify();

  std::vector<ScoreObserver *> m_observers;
  int m_score = 0;
};
