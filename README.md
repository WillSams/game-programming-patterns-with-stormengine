# Game Programming Patterns

Worked examples from Robert Nystrom's
[*Game Programming Patterns*](https://gameprogrammingpatterns.com/), implemented
in C++ on top of [**Storm Engine v2**](https://github.com/WillSams/storm-engine-v2).

Each pattern is a self-contained example under `patterns/`, with its own engine
shell (`Game` + `GameStateMachine` + a `PlayState`), [igloo](https://github.com/joakimkarlsson/igloo)
specs, and README.

## The Patterns

| Pattern | Status | Reference |
|---|---|---|
| [Command](./patterns/command/README.md) | ✅ | [chapter](https://gameprogrammingpatterns.com/command.html) |
| [Flyweight](./patterns/flyweight/README.md) | 🚧 planned | [chapter](https://gameprogrammingpatterns.com/flyweight.html) |

## Requirements

- **Storm Engine v2** installed (`libstormenginev2` + `stormengine2` headers)
- SDL2, SDL2_image, SDL2_ttf, SDL2_mixer
- [igloo](https://github.com/joakimkarlsson/igloo) (header-only test framework)

## Build, run, test

Each pattern builds independently from its own directory:

```bash
cd patterns/command
make            # builds the example into ../../bin/
make run
make test       # builds the igloo specs
make run-test
```

## Adding a new pattern

1. Copy `patterns/__template__/` to `patterns/<name>/`.
2. Replace `REPLACEME` in `Makefile` (`NAME`), `src/game.cpp` window title, and
   the README (including its reference link).
3. Implement the pattern in `src/states/playState.cpp` (and add classes under
   `src/`), with specs under `specs/`.
4. Add a row to the table above.

The shared build config lives in [`common.mk`](./common.mk).
