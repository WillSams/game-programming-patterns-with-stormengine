# Game Loop Pattern

> Reference: [Game Programming Patterns — Game Loop](https://gameprogrammingpatterns.com/game-loop.html)

A game loop runs continuously: process input, update the world, render — decoupled
from the player's input rate and (crucially) from the hardware's clock speed. The
naive loop updates once per frame, so the game runs faster on faster machines.

The robust answer is a **fixed timestep with render interpolation**: advance the
simulation in fixed-size steps (so physics is deterministic and frame-rate
independent), running as many catch-up steps as real time demands, then render by
interpolating between the last two steps using the leftover time (`alpha`). A cap
on steps-per-frame avoids the **spiral of death**, where a slow frame queues more
work than the next frame can clear.

Storm Engine v2 already owns the outer loop; this example implements the pattern's
testable core — the accumulator — and demonstrates the payoff visually.

## How it works

- `FixedTimestep` (`src/loop/fixedTimestep.h`) is the pattern, isolated: feed it a
  frame's elapsed time via `advance()`, and it returns a `StepPlan { updates, alpha }`
  — how many fixed logic steps to run and the interpolation factor to render with.
  It caps `updates` and discards the backlog past the cap.
- `Lerp(previous, current, alpha)` interpolates between two fixed states.
- `PlayState` simulates a bouncing ball at a deliberately low fixed rate (10 Hz)
  while rendering at full frame rate, interpolating its position — smooth motion
  from a chunky simulation. Toggle interpolation off to see the ball jump a whole
  step at a time. The current update rate and interpolation state are drawn on
  screen with a tiny built-in pixel font (`src/ui/pixelText.h`, no font asset).

## Controls

| Key | Action |
|---|---|
| Space | Toggle interpolation (smooth vs. stepped) |
| Up / Down | Raise / lower the fixed update rate |
| R | Reset the ball |
| Esc | Quit |

## Build, run, test

```bash
make            # builds the example into the repo-root bin/
make run
make test       # igloo specs for FixedTimestep and Lerp
make run-test
```
