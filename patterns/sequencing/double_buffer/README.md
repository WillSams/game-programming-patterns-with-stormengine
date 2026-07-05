# Double Buffer Pattern

> Reference: [Game Programming Patterns — Double Buffer](https://gameprogrammingpatterns.com/double-buffer.html)

The double buffer pattern keeps a **pair of buffers**: readers see a stable
`current` buffer while a writer builds the `next` one. A single `swap()` publishes
the finished buffer atomically, so no one ever observes a half-updated state.

The classic case is a graphics framebuffer (draw the frame off-screen, then flip
so the monitor never shows a partial frame — SDL does this for us behind
`SDL_RenderPresent`). But the pattern applies to **any state whose update reads
its own previous values**. This demo uses **Conway's Game of Life**: every cell's
next state depends on its neighbors' *current* state, so the next generation has
to be written into a second grid — mutating in place would corrupt neighbor reads
that haven't happened yet.

Built on **Storm Engine v2**: `Game` runs the loop via a `GameStateMachine`, and
the demo lives in `PlayState`.

## How it works

- `DoubleBuffer<T>` (`src/util/doubleBuffer.h`) is the pattern itself, isolated
  and generic: `current()` reads, `next()` is written, `swap()` flips them.
- `Grid` (`src/life/grid.h`) is a bounded live/dead cell grid.
- `Step(in, out)` (`src/life/life.h`) computes one generation, reading `in` and
  writing `out` — it can't touch `in`, which is exactly why the second buffer
  exists.
- `PlayState` holds a `DoubleBuffer<Grid>`, and each generation does
  `Step(current, next); swap();`, then renders `current()`.

## Controls

| Key | Action |
|---|---|
| Space | Pause / resume |
| N | Single-step one generation (while paused) |
| R | Reseed with a fresh random soup |
| Esc | Quit |

## Build, run, test

```bash
make            # builds the example into the repo-root bin/
make run
make test       # igloo specs for DoubleBuffer<T> and the Life rules
make run-test
```
