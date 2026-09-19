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
| [Bytecode](./patterns/behavioral/bytecode/README.md) | ✅ | [chapter](https://gameprogrammingpatterns.com/bytecode.html) |
| [Subclass Sandbox](./patterns/behavioral/subclass_sandbox/README.md) | ✅ | [chapter](https://gameprogrammingpatterns.com/subclass-sandbox.html) |
| [Type Object](./patterns/behavioral/type_object/README.md) | ✅ | [chapter](https://gameprogrammingpatterns.com/type-object.html) |

### Decoupling Patterns — `patterns/decoupling/`

| Pattern | Status | Reference |
|---|---|---|
| [Component](./patterns/decoupling/component/README.md) | ✅ | [chapter](https://gameprogrammingpatterns.com/component.html) |
| [Event Queue](./patterns/decoupling/event_queue/README.md) | ✅ | [chapter](https://gameprogrammingpatterns.com/event-queue.html) |
| [Service Locator](./patterns/decoupling/service_locator/README.md) | ✅ | [chapter](https://gameprogrammingpatterns.com/service-locator.html) |

### Optimization Patterns — `patterns/optimization/`

| Pattern | Status | Reference |
|---|---|---|
| [Data Locality](./patterns/optimization/data_locality/README.md) | ✅ | [chapter](https://gameprogrammingpatterns.com/data-locality.html) |
| [Dirty Flag](./patterns/optimization/dirty_flag/README.md) | ✅ | [chapter](https://gameprogrammingpatterns.com/dirty-flag.html) |
| [Object Pool](./patterns/optimization/object_pool/README.md) | ✅ | [chapter](https://gameprogrammingpatterns.com/object-pool.html) |
| Spatial Partition | 🚧 planned | [chapter](https://gameprogrammingpatterns.com/spatial-partition.html) |

## Requirements

- **Storm Engine v2 — nothing to install.** It is a submodule at
  `external/storm-engine-v2`, pinned to the revision these examples are written
  against, and the build compiles it from there. Clone with submodules:

  ```bash
  git clone --recurse-submodules <this repo>
  ```

  Already cloned without them? `make engine` fetches and builds the pinned
  revision. Nothing is installed system-wide, and the build deliberately has no
  "use the system engine instead" path — that is how a local build and CI came to
  run different engines (2.3.0 here, 2.3.1 in CI) without either saying so.
- SDL2, SDL2_image, SDL2_ttf, SDL2_mixer
- [igloo](https://github.com/joakimkarlsson/igloo) (header-only test framework)

### Staying up to date with the engine

The submodule pins a **commit**, so a build is reproducible until someone moves
it on purpose:

```bash
make engine-update    # advance the pin to the engine's newest main, and rebuild
make engine-version   # which revision is pinned
```

An automatic "always newest" was rejected: a red build should be traceable to a
commit in *this* repo, not to one that landed in the engine five minutes ago.

## Build, run, test

The font the demos draw with is **shared** (`include/pixelFont.h` + its SDL
drawer), and its spec lives in `patterns/__shared__/` — that folder is not a
pattern; `__template__` is the one to copy.

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
root from its own path, so patterns work at any folder depth. It builds against
the pinned engine via [`engine.mk`](./engine.mk), and force-includes
[`include/engineGlobal.h`](./include/engineGlobal.h) — a bridge that puts the
engine's `namespace storm` names back in the global scope so the pattern code
reads `GameState` rather than `storm::GameState`.
