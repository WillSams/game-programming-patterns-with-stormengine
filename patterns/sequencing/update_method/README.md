# Update Method Pattern

> Reference: [Game Programming Patterns — Update Method](https://gameprogrammingpatterns.com/update-method.html)

Every game object that lives over time gets an `update()` method, called once per
frame. Each object advances *its own* behavior by one time slice and keeps whatever
state it needs between frames. A single loop over a collection of objects drives
them all — so a patrolling guard and a periodic turret each mind their own logic
without one monster update function that knows about every entity type.

Built on **Storm Engine v2**: `Game` runs the loop via a `GameStateMachine`, and
the demo lives in `PlayState`.

## How it works

This mirrors the book's courtyard example — patrolling skeleton guards and
enchanted statues that periodically shoot lightning — with real delta-time instead
of the chapter's naive frame counter (frame-rate independent, and consistent with
the [Game Loop](../game_loop/README.md) example).

- `Entity` (`src/entities/entity.h`) is the interface: a pure virtual `update(dt)`
  plus a position. (Namespaced `um::` so it doesn't collide with the engine's own
  ECS `Entity`.)
- `Skeleton` (`src/entities/skeleton.h`) paces between two x bounds, carrying its
  heading across frames and reversing at each wall.
- `Statue` (`src/entities/statue.h`) is stationary and shoots every `period`
  seconds, tracking its own timer and shot count (and a decaying `glow` for the
  render flash).
- `World` (`src/entities/world.h`) holds the entities and its `update(dt)` is the
  pattern itself: one loop calling `update()` on each. New entity types need no
  change here.

`PlayState` builds a `World` of several skeletons (each its own lane and speed)
and statues (each its own period), then advances them all with a single
`world.update(dt)` every frame. Statues flare bright the instant they fire.

## Controls

| Key | Action |
|---|---|
| Esc | Quit |

## Build, run, test

```bash
make            # builds the example into the repo-root bin/
make run
make test       # igloo specs for Skeleton, Statue, and World
make run-test
```
