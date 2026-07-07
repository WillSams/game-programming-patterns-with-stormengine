# Game Programming Patterns

Worked examples from Robert Nystrom's
[*Game Programming Patterns*](https://gameprogrammingpatterns.com/), implemented
in C++ on top of [**Storm Engine v2**](https://github.com/WillSams/storm-engine-v2).

Each pattern is a self-contained example under `patterns/<category>/<name>/`,
with its own engine shell (`Game` + `GameStateMachine` + a `PlayState`),
[igloo](https://github.com/joakimkarlsson/igloo) specs, and README. Categories
follow the book's structure.

## The Patterns

### Design Patterns Revisited — `patterns/design/`

| Pattern | Status | Reference |
|---|---|---|
| [Command](./patterns/design/command/README.md) | ✅ | [chapter](https://gameprogrammingpatterns.com/command.html) |
| [Flyweight](./patterns/design/flyweight/README.md) | ✅ | [chapter](https://gameprogrammingpatterns.com/flyweight.html) |
| [Observer](./patterns/design/observer/README.md) | ✅ | [chapter](https://gameprogrammingpatterns.com/observer.html) |
| [Prototype](./patterns/design/prototype/README.md) | ✅ | [chapter](https://gameprogrammingpatterns.com/prototype.html) |
| [Singleton](./patterns/design/singleton/README.md) | ✅ | [chapter](https://gameprogrammingpatterns.com/singleton.html) |
| [State](./patterns/design/state/README.md) | ✅ | [chapter](https://gameprogrammingpatterns.com/state.html) |

### Sequencing Patterns — `patterns/sequencing/`

| Pattern | Status | Reference |
|---|---|---|
| [Double Buffer](./patterns/sequencing/double_buffer/README.md) | ✅ | [chapter](https://gameprogrammingpatterns.com/double-buffer.html) |
| [Game Loop](./patterns/sequencing/game_loop/README.md) | ✅ | [chapter](https://gameprogrammingpatterns.com/game-loop.html) |
| [Update Method](./patterns/sequencing/update_method/README.md) | ✅ | [chapter](https://gameprogrammingpatterns.com/update-method.html) |

### Behavioral Patterns — `patterns/behavioral/`

| Pattern | Status | Reference |
|---|---|---|
| Bytecode | 🚧 planned | [chapter](https://gameprogrammingpatterns.com/bytecode.html) |
| Subclass Sandbox | 🚧 planned | [chapter](https://gameprogrammingpatterns.com/subclass-sandbox.html) |
| Type Object | 🚧 planned | [chapter](https://gameprogrammingpatterns.com/type-object.html) |

### Decoupling Patterns — `patterns/decoupling/`

| Pattern | Status | Reference |
|---|---|---|
| Component | 🚧 planned | [chapter](https://gameprogrammingpatterns.com/component.html) |
| Event Queue | 🚧 planned | [chapter](https://gameprogrammingpatterns.com/event-queue.html) |
| Service Locator | 🚧 planned | [chapter](https://gameprogrammingpatterns.com/service-locator.html) |

### Optimization Patterns — `patterns/optimization/`

| Pattern | Status | Reference |
|---|---|---|
| Data Locality | 🚧 planned | [chapter](https://gameprogrammingpatterns.com/data-locality.html) |
| Dirty Flag | 🚧 planned | [chapter](https://gameprogrammingpatterns.com/dirty-flag.html) |
| Object Pool | 🚧 planned | [chapter](https://gameprogrammingpatterns.com/object-pool.html) |
| Spatial Partition | 🚧 planned | [chapter](https://gameprogrammingpatterns.com/spatial-partition.html) |

## Requirements

- **Storm Engine v2** installed (`libstormenginev2` + `stormengine2` headers)
- SDL2, SDL2_image, SDL2_ttf, SDL2_mixer
- [igloo](https://github.com/joakimkarlsson/igloo) (header-only test framework)

## Build, run, test

Each pattern builds independently from its own directory (binaries land in the
repo-root `bin/`):

```bash
cd patterns/design/command
make            # builds the example
make run
make test       # builds the igloo specs
make run-test
```

## Adding a new pattern

1. Copy `patterns/__template__/` to `patterns/<category>/<name>/`.
2. In its `Makefile`, set `NAME` and make the `include` path point at the root
   `common.mk` (`../../../common.mk` for a `<category>/<name>` folder).
3. Replace `REPLACEME` in `src/game.cpp` (window title) and the README
   (including its reference link).
4. Implement the pattern in `src/states/playState.cpp` (and add classes under
   `src/`), with specs under `specs/`.
5. Flip the pattern's status to ✅ above.

The shared build config lives in [`common.mk`](./common.mk); it locates the repo
root from its own path, so patterns work at any folder depth.
