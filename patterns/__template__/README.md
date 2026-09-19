# REPLACEME Pattern

Starter for a Game Programming Patterns example built on **Storm Engine v2**.

## Layout

```text
main.cpp              Entry point — constructs Game, runs the loop
src/game.h / .cpp     Engine shell: window, renderer, GameStateMachine
src/states/           One GameState per screen; PlayState is the demo canvas
specs/                igloo specs (specs/main.cpp is the runner)
```

Keep the **pattern's logic in a header under `src/` that has no dependency on
SDL** — `src/<pattern>/someRule.h` — and let `PlayState` be the shell that drives
it. A spec can then reach the logic directly, headless.

## Build & run

```bash
make        # builds the example into the repo-root bin/
make run
```

## Test

```bash
make test       # BUILDS the igloo specs (it does not run them)
make run-test   # runs them
```

⚠️ Two steps, not one. A change verified with only `make test` has proved the
specs compile.

## Creating a new pattern

1. Copy this folder to `patterns/<category>/<name>/` (categories: `design`,
   `sequencing`, `behavioral`, `decoupling`, `optimization`).
2. ⚠️ Change the `Makefile`'s include from `../../common.mk` — correct at this
   template's own depth, `patterns/__template__/` — to `../../../common.mk`.
   Forget it and the first `make` fails.
3. Replace `REPLACEME` in the `Makefile` (`NAME`) and in `src/game.cpp`'s window
   title, and rewrite this README.
4. Implement the pattern, with its testable core in a header under `src/` and
   specs under `specs/`.
5. Flip that pattern's existing `🚧 planned` row in the root README to ✅ and link
   it — convert the row in place, do not add a second table.

The engine is the pinned submodule at `external/storm-engine-v2`; nothing is
installed system-wide. `make engine` builds it.