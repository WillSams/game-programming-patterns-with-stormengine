# Game Programming Patterns — on Storm! Engine v2

Worked examples from Robert Nystrom's *Game Programming Patterns*, in C++17 on
[Storm! Engine v2](https://github.com/SamsWebs/storm-engine-v2). **This is a
teaching repo, not a game.** Each pattern is a self-contained folder that a
reader can open on its own: `patterns/<category>/<name>/`, with its own engine
shell, igloo specs, README and Makefile.

Debt here is therefore *"the on-ramp is wrong"* or *"a build can't be
reproduced"*, never *"these files look alike"*. Nine patterns deliberately repeat
the same shell, and that is the affordance, not duplication to extract.

## The engine is THE SUBMODULE, always

**Storm! Engine v2 is `external/storm-engine-v2`, and the build uses that
revision — never a system install.**

- `make engine` — fetch and build the pinned revision (also happens
  automatically: every target depends on the engine library).
- `make engine-version` — which revision is pinned.
- `make engine-update` — advance the pin to the engine's newest `main` and
  rebuild. **Run this when the engine changes; this repo tracks it.**
- The pin is a **commit**, so a build is reproducible until someone moves it on
  purpose. An automatic "always newest" was rejected: a red build must be
  traceable to a commit here, not to one that landed in the engine minutes ago.

⚠️ **There is deliberately NO fallback to a system-installed engine.** There was
one, and it is how this repo and CI silently ran different engines — **2.3.0
installed locally, 2.3.1 in CI** — with nothing in either place saying which was
in play. A fallback is exactly how the two quietly diverge, so it is gone.

Two consequences worth knowing:

- **A from-source build needs ONE nested submodule**: the engine compiles
  tinyxml2 *in* (`vendor/android/tinyxml2`), so it fails without it. It is nested,
  so it can only be initialised from **inside** the engine —
  `git -C external/storm-engine-v2 submodule update --init vendor/android/tinyxml2`.
  Addressing it from this repo fails with `pathspec ... did not match any file(s)`.
- **The engine builds in-tree**, so the submodule reads dirty afterwards.
  `.gitmodules` sets `ignore = dirty` for that reason. Without it every
  `git status` here shows the engine as modified, which trains you to ignore the
  one line that matters.

## The bridge: `include/engineGlobal.h`

Engine 2.0.0 moved every type into `namespace storm`; the pattern code is written
unqualified (`class PlayState : public GameState`). The bridge restores the names
and is **force-included from `common.mk`** (`-include engineGlobal.h`).

⚠️ **It is NOT the engine's own `stormengine2/compat/global.h`.** That shim
includes the whole engine by design, dragging `ecs.h`, `xmlLoader.h` and tinyxml2
into all ~113 files so four names cost the entire engine. The bridge here names
**only the four headers and seven names the patterns actually use**.

**Derive that list; do not remember it.** `Entity` is the trap: it appears in four
files and is *pattern-local* (`update_pattern/src/entities/` defines its own
`class Entity`), so exporting the engine's would collide with it.

```sh
grep -rhoE '#include <stormengine2/[^>]+>' patterns/ | sort | uniq -c   # the headers
grep -rl '\bAssetStore_Ptr\b' patterns/ --include='*.h' --include='*.cpp'  # the names
```

**The bridge exists to be deleted** — the migration ends by qualifying the names
or adding `using namespace storm;` per file, then dropping the force-include.

## Build, test, and the one gotcha

```sh
cd patterns/<category>/<name>
make          # build the example (binaries land in the repo-root bin/)
make run
make test     # BUILDS the igloo specs
make run-test # RUNS them  <-- two steps, not one
```

⚠️ **`make test` does not run anything.** It builds `bin/test-<name>`; `make
run-test` executes it. A change verified with only `make test` has proved that the
specs *compile*. CI runs all three — `make`, `make test`, `make run-test` — in that
order; do the same.

⚠️ **AND TWO TRAPS IN `common.mk`, both of which shipped and both of which CI was
blind to:**

  * **THE DEFAULT GOAL IS STATED, NOT INHERITED.** `common.mk` includes
    `engine.mk`, whose first target is the engine library — so for a while a bare
    `make` built the ENGINE and never the example, for every pattern, while the
    README said `make` builds the example. `.DEFAULT_GOAL := all` names it. A
    default goal that depends on include order breaks in one commit and is noticed
    three later.
  * **`run-test` DEPENDS ON `test-target`.** It used to just execute the binary, so
    it only worked when something had already built it — and `make` cleans, so
    running `make` and then `make run-test` met "Command not found".

Both were found by running the documented commands in the documented order rather
than by running the suite. **CI now runs `make` too**, because the suite alone
cannot see a broken default goal: `make test` was green throughout.

## Writing a pattern

Follow BDD with igloo (`specs/*.spec.cpp`, `Describe`/`It`), and **keep the
pattern's logic in headers that have no dependency on SDL** so a spec can reach
it. The split that falls out of that:

- `src/<pattern>/someRule.h` — **pure**, no SDL, no engine, spec'd directly. This
  is where the pattern's actual idea lives (e.g. `game_loop/src/loop/fixedTimestep.h`).
- `src/states/playState.*` — the SDL/engine shell that drives it. Not spec'd;
  it is the demo's presentation.

A pattern whose logic can only be tested by opening a window is the wrong shape.
**Prefer a pure header over logic in the state.** The same split applies to a
pattern's UI helpers: `bytecode` keeps its 3x5 font TABLE in a pure
`src/ui/glyphs.h` (spec'd, and see below) with the SDL drawing in
`src/ui/pixelText.h`.

Each pattern carries its own copy of that font, which is the same deliberate
duplication as the shell — the alternative is a shared header that every pattern
depends on, and this repo's examples are meant to be readable one folder at a
time.

⚠️ **A FONT IS A COVERAGE CONTRACT, AND A MISSING GLYPH DRAWS BLANK SILENTLY.**
`bytecode`'s spec asserts every character the demo prints has a visible glyph, and
that no two characters render identically *except* the two conventions it names
(`O`/`0` and `S`/`5`, which collapse in a 3x5 font and are told apart by context).
It found those two the moment it was written.

⚠️ **AND NO TEST CAN SEE THAT AN `N` LOOKS LIKE A `K`.** Two attempts at that
glyph were wrong and the rendered alphabet was the only thing that showed it — the
demo's own label read "SPELL 1 MIKOR HEAL". **M, N and W are approximations at 3x5**
(three columns cannot draw a diagonal). Render the alphabet and read it when you
touch the font.

Steps to add one:

1. Copy `patterns/__template__/` to `patterns/<category>/<name>/`.
2. In its `Makefile`, set `NAME` and point the include at the root `common.mk` —
   `../../../common.mk` for `<category>/<name>` (the template ships `../../` and
   is one level shallower).
3. Replace every `REPLACEME`.
4. Implement it, with the pure part in a header under `src/`.
5. Add its row to the README table and flip the status to ✅.

## Git

- **Branch, then PR. Never on `main`.** `feat/*`, `fix/*`, `docs/*` — the repo's
  own `branch-name-check` enforces the prefixes.
- Signed commits. Commit messages say **why**, in the prose this repo already
  uses; the diff says what.
- `gh` lives at `~/.local/bin/gh` if it is not on `PATH`.

## Writing style

**Gender-neutral.** `they`/`their`, `the reader`, `the player`, the pattern name —
not `he` as the generic. A specific person keeps their own pronouns.
