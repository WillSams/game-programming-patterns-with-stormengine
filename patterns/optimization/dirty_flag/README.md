# Dirty Flag Pattern

> Reference: [Game Programming Patterns — Dirty Flag](https://gameprogrammingpatterns.com/dirty-flag.html)

**Do the work when someone asks, not when something changes.** A parent moves and
every descendant's world position is wrong; recomputing all of them every frame walks
the whole tree for the sake of one node, and recomputing eagerly on every change
walks it every time a node changes. The flag **defers** it: mark the subtree out of
date, and rebuild a node only when something actually reads it.

## How it works

- `DirtyFlag` (`src/dirty/transformTree.h`) is a bool that **starts set** — "never
  computed" is not "up to date", and a flag that starts `false` is the classic bug.
- `TransformTree` holds `parent` and a `local` offset per node, plus a **cached**
  world position that is valid only while the flag is clean.
- `SetLocal` does **no arithmetic at all**: it marks the node and its descendants
  stale and returns. The cost of a change no longer scales with how much depends on it.
- `WorldX`/`WorldY` resolve a stale node on the way out — and resolve its **parent
  first**, or the child inherits a stale ancestor.

### ⚠️ The pattern's real danger is not performance

**It is that the flag is easy to forget to set, and the consequence is a SILENT
WRONG ANSWER.** No crash, no warning — just a stale number, forever. A missed
`MarkSubtree` would fail nothing loudly here; it would draw a box in the wrong place.
So the specs pin both halves:

1. **That the deferral is real**, by counting recomputations — a change costs a mark
   and zero rebuilds; reading a clean node costs nothing; only the *stale* part is
   redone (change one branch of a ten-node tree, read all ten, and exactly **three**
   are rebuilt).
2. **That the cache never lies.** The ground truth is **not** another cached value:
   the spec walks the parent chain and sums the local offsets by hand, and compares.
   That is the case that catches a `SetLocal` which forgot to propagate.

⚠️ `WorldX` is deliberately **not `const`**. A read of a deferred value *is* a
mutation — it brings the cache up to date and counts the work — and a `const` version
would have to either return a stale value (the exact bug) or mutate through a cast.

## Controls

| Key | Action |
|---|---|
| Space | Move the root — marks all 17 nodes, rebuilds none |
| C | Move ONE branch — marks **4 of 17** |
| A | Auto-move on/off |
| R | Clear the counters (the scene is untouched) |
| 0 | Rebuild the scene |
| Esc | Quit |

**Read the two counters.** Press **C** and the column shows `MARKS 1`,
`STALE BEFORE DRAW 4`, `REBUILT 4`, `READS 98`, `AVOIDED 96 PCT` — one change, four
nodes rebuilt out of seventeen. Turn auto-move **off** (A) and the scene keeps being
drawn: `READS` climbs while `REBUILT` stops. That gap is the pattern.

⚠️ **`STALE BEFORE DRAW` is captured before the draw loop**, because *drawing is the
read that resolves.* Read after, and the number would always be zero.

## Build, run, test

```bash
make            # builds the example into the repo-root bin/
make run
make test       # igloo specs: deferral counted, walked ground truth, guarded indices
make run-test
```

⚠️ `make test` only **builds** the specs; `make run-test` runs them.
