#pragma once

// Observer interface: anything that wants to react when the score changes
// implements this. The subject only knows about this interface, not the
// concrete reactions.
class ScoreObserver {
public:
  virtual ~ScoreObserver() = default;
  virtual void onScoreChanged(int newScore) = 0;
};
