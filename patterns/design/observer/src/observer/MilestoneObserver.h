#pragma once

#include "ScoreObserver.h"

// A second concrete observer reacting to the *same* notifications differently:
// it flags whenever the score crosses a new multiple of `interval`.
class MilestoneObserver : public ScoreObserver {
public:
  explicit MilestoneObserver(int interval = 100) : m_interval(interval) {}

  void onScoreChanged(int newScore) override {
    int reached = newScore / m_interval;
    if (reached > m_lastMilestone) {
      m_lastMilestone = reached;
      m_justReached   = true;
    }
  }

  int  milestonesReached() const { return m_lastMilestone; }

  // One-shot read: true once after each newly crossed milestone.
  bool consumeJustReached() {
    bool r = m_justReached;
    m_justReached = false;
    return r;
  }

private:
  int  m_interval;
  int  m_lastMilestone = 0;
  bool m_justReached   = false;
};
