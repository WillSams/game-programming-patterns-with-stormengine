# Spatial Partition Pattern

> Reference: [Game Programming Patterns — Spatial Partition](https://gameprogrammingpatterns.com/spatial-partition.html)

**Ask the cell, not the world.** A query that wants to know what is near a place
reads every object in the world. Put the objects into buckets and the same query
reads only the objects that are there. The arithmetic per pair does not change —
the number of pairs does.

## How it works

- `Grid` (`src/partition/grid.h`) is a fixed `NxN` lattice over the unit square,
  one bucket per cell, and **every entity in exactly one bucket**.
- `CellIndex` maps a position to a cell, and returns `-1` for a position outside the
  world — it does **not** clamp to an edge cell, which would file an escaped entity
  into a bucket it is nowhere near.
- `Move` is the operation the pattern gets wrong, and the one the specs are built
  around: an entity that stays in its cell costs **no bookkeeping at all**, and one
  that crosses is detached from the old bucket before it is added to the new one.
- `CandidatesAround(x, y, ring, out)` is the chapter's melee query — the
  `(2·ring+1)²` block of cells around a position. `ring = 0` is "who is in this
  cell", which is what the demo asks.

### ⚠️ The pattern's silent bug is a stale bucket

An entity that crosses into a new cell but is never taken out of the old one is
still found **where it used to be**, and not found where it is. Nothing crashes,
nothing warns — a query just returns the object from the wrong place. It is the
same shape of failure as a missed dirty mark.

So the specs check placement after a **crossing**, not only after an insert, and
check the invariant that makes every other answer trustworthy: after 200 moves
along bouncing paths, the bucket sizes still sum to the number of entities in the
grid, and each one is findable at its current position. A duplicate is impossible
by construction — an entity is in one bucket and the block visits each bucket once
— which is exactly why a duplicate would mean the placement is broken rather than
something to filter out in the query.

⚠️ **A fixed lattice is the trade, not a defect.** The cells never adapt, and a bad
cell size is the cost of choosing this pattern. Adaptive subdivision is a tree,
which is a different chapter.

## Controls

| Key | Action |
|---|---|
| A | Entity drift on/off |
| P | Park the probe where it is |
| G | Grid lines on/off — the buckets are still there |
| R | Clear the counters (the world is unchanged) |
| 0 | Rebuild the world |
| Esc | Quit |

**Read the right column.** `IN CELL` is what a query touches and `ALL` is what the
same query costs without a partition — usually 2–5 against 240, so `SKIPPED` sits
near 98%. The bright dots are exactly the list the partition returned; the dim ones
are entities the query never looked at.

`MOVES` is one per entity per frame, and `CROSSINGS` is how many of those actually
changed bucket — around a quarter. **The gap between them is the work the partition
did not do**, and it is why keeping a grid current is cheap.

## Build, run, test

```bash
make            # builds the example into the repo-root bin/
make run
make test       # igloo specs: the crossing, the invariant, degenerate grids
make run-test
```

⚠️ `make test` only **builds** the specs; `make run-test` runs them.