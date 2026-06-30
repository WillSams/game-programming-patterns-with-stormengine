# Prototype Pattern

> Reference: [Game Programming Patterns — Prototype](https://gameprogrammingpatterns.com/prototype.html)

The prototype pattern creates new objects by **cloning an existing instance**
instead of instantiating a class by name. A `Spawner` holds one prototype
monster and makes more by calling `clone()` on it — no separate spawner class
per monster type, and swapping the prototype changes what gets produced.

Built on **Storm Engine v2**: `Game` runs the loop via a `GameStateMachine`, and
the demo lives in `PlayState`.

## How it works

- `Monster` declares `clone()` (and carries intrinsic stats: health, speed,
  colour). Each concrete monster (`Ghost`, `Demon`, `Sorcerer`) clones itself
  via its copy constructor.
- `Spawner` owns a `std::unique_ptr<Monster>` prototype; `spawn()` returns
  `prototype->clone()`. `setPrototype()` swaps it at runtime.
- Every spawn is an independent copy — changing one clone never affects the
  prototype or its siblings.

## Controls

| Key | Action |
|---|---|
| 1 / 2 / 3 | Set the prototype to Ghost (white) / Demon (purple) / Sorcerer (red) and spawn one |
| Space | Spawn another clone of the current prototype |
| Esc | Quit |

The swatch in the top-left shows the current prototype; spawned clones drift and
bounce, coloured by type.

## Build, run, test

```bash
make            # builds the example into the repo-root bin/
make run
make test       # igloo specs for cloning and the spawner
make run-test
```
