# Tech Debt — Status: 1 open item, none in the demo code

Last reviewed: 2026-11-15 — first re-audit, after the engine became a submodule
and the pattern count grew from 9 to 17.

**The pattern code is still healthy.** 10,342 lines across 187 headers and
sources in 17 demos, largest file 253 lines
(`patterns/optimization/data_locality/src/states/playState.cpp`), no raw
`new`/`delete` outside the one `new PlayState(...)` the engine's
`GameStateMachine` requires, and no throwing constructs reachable from any
header. Every finding is in the build, the CI, or the docs — the scaffolding
around the demos, not the demos.

This is a teaching repo, not a shipped game. Debt is therefore *"the on-ramp is
wrong"* or *"a build can't be reproduced"*, never *"these files look alike"* —
see [What to watch](#what-to-watch) for the things that look like debt and are
not.

## Summary

| Priority | Focus | Status |
|----------|-------|--------|
| P1 | CI container image is the last unpinned input (`:latest`) | 🔴 Open |

Everything the first audit filed is now closed. That audit predated the engine
submodule, and three of its six items were consequences of the system-installed
engine; the rest were fixed in the re-audit commit described below.

## Open item

### P1 — The CI container image is unpinned

**Category:** structural rot (build reproducibility)
**Evidence:** `.github/workflows/pr-validate.yml:19` —
`container: storminator16/igloo-testing:latest`. The workflow deliberately pins
everything else now: the engine is the submodule at a commit
(`external/storm-engine-v2`, `heads/main`), and `actions/checkout` and
`actions/github-script` are on major tags. The container is a floating tag.

This is the exact failure the engine pin was introduced to kill, one layer up.
`igloo` is *installed in the image*, and the image's SDL / tinyxml2 sonames are
what the engine library links against, so republishing that tag changes a
build input with no commit in this repo. A green PR today and a red one tomorrow
with nothing between is the thing a teaching repo cannot afford, because a
reader has no way to tell which side moved.

**Risk:** Low frequency, high confusion. The failure would present as a link
error about a soname, in a diff that touched none of it, and the maintainer of
the image is the same person who owns this repo — so it will happen during an
unrelated engine bump and be blamed on that.

**Fix:** Pin the image to a digest
(`storminator16/igloo-testing@sha256:...`), or publish versioned tags
(`:1`, `:2026-11`) and reference one. Dependabot can bump a digest. Same
argument as the engine pin: a build should be traceable to a commit here.

**Cost:** One line, once a digest or version tag exists to point at.

## Fixed since the first audit

| Was | Found | Fixed by |
|-----|-------|----------|
| P1 | Storm Engine v2 unpinned; CI downloaded the newest `.deb` at run time | Engine is now the submodule `external/storm-engine-v2` at a pinned commit, built from source; `common.mk`/`engine.mk` have no system-install fallback, and CI checks the submodule out with `submodules: true` |
| P2 | `__template__` README said gtest, `patterns/<name>/`, `../../bin` | Rewritten: igloo, `patterns/<category>/<name>/`, repo-root `bin/`, the `../../../common.mk` include-depth step, and the pattern-core-in-a-pure-header split. The three pattern READMEs repeating the stale `../../bin` path (`command`, `flyweight`, `observer`) were corrected |
| P3 | CI ran `make test` and `make run-test` but never `make` | `.github/workflows/pr-validate.yml` now runs all three, `make` first, for every folder with a Makefile |
| P4 | `make` wiped the shared repo-root `bin/` and forced a full rebuild | `common.mk` no longer makes `all` depend on `clean`; `clean` is scoped to this pattern's own target, test target and objects. The root Makefile keeps the wipe-everything escape hatch. The duplicate `test: test-target` rule was deleted |
| P5 | Dependabot auto-merged **major** action bumps despite the step name | The `semver-major` clause is gone from `dependabot-auto-merge.yml`; the step now does what it is called |
| P6 | Stale `graphics/` and `.o` files in the installed engine include tree | **Withdrawn, not fixed.** There is no system-installed engine to audit any more; the submodule's `common/` is the only include tree and it has no build artifacts in it. The packaging residue, if it still exists at `/usr/local`, now affects nobody here |

## What to watch

Examined and deliberately left alone. Reasons given so the next audit does not
re-file them.

- **Duplication across demos — not debt, by design.** The per-demo engine shell
  (`src/game.cpp`, `src/game.h`, the `PlayState` declaration, the frame-pacing
  block) is repeated in every demo on purpose. A reader opening
  `patterns/design/command/` must see the whole program — window, renderer,
  `GameStateMachine`, state — without following an include into a shared
  library. Self-containedness is the product. **What would change this:** if a
  shell change ever has to be applied 17 times *and* getting it wrong fails
  silently. Today it fails at compile time, in one demo. The one thing that is
  *not* a template is the 3×5 font, and it is already shared
  (`include/pixelFont.h`, spec in `patterns/__shared__/`) precisely because it
  is one decision rather than a shell.

- **Only 4 of the engine's headers are used — also fine.** `assetStore.h`,
  `logger.h`, `gameStateMachine.h`, `states/gameState.h`. The ECS, the systems,
  the loaders, `input/` and `net/` are untouched. That is the correct ratio for
  a pattern demo: it needs a window and a loop, not an entity system. (The
  engine's `Entity` is a trap anyway — the `update_method` demo defines its own
  `Entity`, and the bridge in `include/engineGlobal.h` deliberately does not
  export the engine's.)

- **Demos that teach what the engine supersedes — already acknowledged.** Update
  Method builds a hand-rolled `Entity` with a virtual `update(dt)` where the
  engine ships an ECS; Game Loop implements an accumulator where
  `GameStateMachine` already owns the outer loop. Both are the point of their
  chapters, and both READMEs say so.

- **The engine's `GameState::millisecondsPreviousFrame` is dead in every demo.**
  Each declares its own `millisecondsPreviousFrame_` beside it — not shadowing,
  the names differ by the trailing underscore — so it compiles cleanly and the
  demos are internally consistent. Left alone because changing it touches 17
  files to save 17 ints. Reconsider only if the engine ever *writes* that member.

- **Indentation is split: 113 files at 4 spaces, 31 at 2, 25 tab-indented**, and
  there is still no `.clang-format` in the repo. The 2-space files are the
  `design/` pattern cores — engine house style; the rest are shells and specs.
  Compounding it, `.vscode/settings.json:24` sets `editor.formatOnSave: true`
  with no format config, so VS Code applies its LLVM default (2-space) and
  silently reformats 4-space files on open-and-save. Not filed as an item: it
  changes no behaviour, and a repo-wide reformat would bury the history of a
  repo whose history *is* the teaching material. **What would change it:** add a
  `.clang-format` and reformat once, in a single isolated commit, before the next
  batch of patterns lands.

- **3 of 6 assets are unreferenced** — `world.png`, `player.png`,
  `johnny-rezende-sheet-dante.jpg`, survivors of the pre-Storm-Engine version of
  this repo. Only the flyweight demo loads assets at all (`grass.png`,
  `water.png`, `hill.png`). Harmless; delete them whenever someone is already in
  `assets/`.

- **`.vscode/` is stale.** `settings.json:5,11,17` colour `googletest.*` result
  tokens — a gtest fossil like the template README had — and `tasks.json`
  compiles the active file with bare `g++ ${file}`, with no include path to the
  submodule, no SDL and no `-lstormenginev2`, so it cannot build any file here.
  `settings.json:87` also leaves a trailing comma before the closing brace
  (valid JSONC, invalid JSON). Cosmetic; nobody has edited it since 2023.

- **`SRCS = $(wildcard src/**/*.cpp)` only reaches one level deep.** GNU make's
  `wildcard` has no recursive glob; `src/**/*.cpp` expands as `src/*/*.cpp`.
  Every pattern today is at most `src/<dir>/*.cpp`, so nothing is missed, and
  `find patterns -path '*src/*/*/*.cpp'` returns nothing. A future pattern
  nesting sources two levels down would have them silently dropped and surface
  as an undefined reference at link. Same expression in the spec build.

- **No portability findings, because the checks do not apply.** There is no
  `Makefile.nx` and no `Makefile.android`; desktop Linux is the only target.
  For the record: `grep` for `.at(`, `std::sto*`, `throw` and `try {` across
  `patterns/**/*.h` returns only comments claiming SDL-freedom, so there are no
  throwing constructs reachable from headers. SDL appears in headers exactly
  where it should — the `game.h`/`playState.h` shells, `command`'s
  `InputHandler.h` (an `SDL_Keycode` map) and the shared `pixelText.h` drawer.
  Every pattern *core* header is SDL-free, which is why each has a spec that
  runs headless.

- **No reinvention findings.** Nothing under `patterns/` rebuilds an engine
  subsystem. The demos that resemble engine features resemble them because the
  chapter is about that feature.

## Next step

P1: pin the CI container image to a digest or a versioned tag. It is the last
input to a build that is not traceable to a commit, and it is one line.