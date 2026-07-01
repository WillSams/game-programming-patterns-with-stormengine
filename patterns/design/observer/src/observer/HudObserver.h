#pragma once

#include "ScoreObserver.h"

// A concrete observer that simply remembers the latest score so the HUD can
// draw it. It reacts to the change without the subject knowing it exists.
class HudObserver : public ScoreObserver {
public:
  void onScoreChanged(int newScore) override { m_displayed = newScore; }
  int displayed() const { return m_displayed; }

private:
  int m_displayed = 0;
};
