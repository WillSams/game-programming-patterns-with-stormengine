# REPLACEME Pattern

Starter for a Game Programming Patterns example built on **Storm Engine v2**.

## Layout

```text
main.cpp              Entry point — constructs Game, runs the loop
src/game.h / .cpp     Engine shell: window, renderer, GameStateMachine
src/states/           One GameState per screen; PlayState is the demo canvas
specs/                gtest specs (specs/main.cpp is the runner)
```

## Build & run

```bash
make        # builds ../../bin/REPLACEME_pattern_example
make run
```

## Test

```bash
make test       # builds ../../bin/test-REPLACEME_pattern_example (gtest/gmock)
make run-test
```

## Creating a new pattern

1. Copy this folder to `patterns/<name>/`.
2. Replace `REPLACEME` in `Makefile` (`NAME`), `main.cpp`/`game.cpp` window title,
   and this README.
3. Implement the pattern inside `src/states/playState.cpp` (and add
   components/systems under `src/`), with specs under `specs/`.

Requires Storm Engine v2 installed (`libstormenginev2` + `stormengine2` headers).
