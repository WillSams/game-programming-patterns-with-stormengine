# Data Locality Pattern

> Reference: [Game Programming Patterns — Data Locality](https://gameprogrammingpatterns.com/data-locality.html)

**The problem is not a slow algorithm — it is an algorithm that spends its time
waiting.** A loop over an array of pointers touches one object per cache line, so
every element is a miss; the same loop over contiguous data walks the line by line
and misses once per sixteen elements. Nothing about the arithmetic changes. The
**layout** does.

## How it works

- `ParticleHot` (`src/locality/particles.h`) is **exactly its four floats** — every
  byte here is a byte pulled into cache for every element.
- `ParticleCold` is the name, colour and visibility: read when *drawing*, and by
  nothing in the loop.
- `MixedWorld` keeps one array of one struct with hot and cold fields **together**:
  each element's cache line is mostly data the loop will not read.
- `SplitWorld` keeps the hot array and the cold array **side by side**, indexed the
  same way. Its hot loop reads `hot_` and nothing else.

Both layouts share **one** `StepParticle`, because two copies of the update rule is
how an "optimization" silently becomes a different simulation.

### ⚠️ What a spec can and cannot pin here

**A test cannot assert that memory is faster.** That is a fact about a machine, not
about a suite, and a timing assertion is a flake generator. So the specs pin the two
deterministic halves:

1. **The optimization does not change the world.** Both layouts are stepped and
   compared. The comparison is deliberately tolerant in the last bit — a compiler
   may vectorize one loop and contract a multiply-add in the other, and insisting on
   exact equality would blame the code for the compiler's arithmetic.
2. **The layout is what the pattern claims**, checked rather than trusted:
   `sizeof(ParticleHot) == 4 * sizeof(float)`; consecutive elements are **exactly one
   struct apart**; and the cold array is **byte-identical after a hot update** — the
   actual claim of the pattern ("the hot loop did not drag cold data in"), which a
   timing number cannot show.

The screen shows the measurement. **The specs prove the fast one is not a different
world.**

## The cost the pattern adds — not hidden

A split world's cold data must be reunited with its hot data **through a shared
index**. The demo's field read both arrays per dot, which is the reunion the specs
pin, and it is why a split layout is not simply "better": the loop got cheaper and
the reader got an index to keep.

## Controls

| Key | Action |
|---|---|
| 1 / 2 | 200 / 2000 particles |
| B | Time both layouts (12 passes each, after warming both up) |
| R | Reset the field |
| Esc | Quit |

Press **B** and read the column: `HOT BYTES 16` against `MIXED BYTES 56`, then
`MIXED` and `SPLIT` as **nanoseconds per particle-step** and the ratio. `SUM MATCH`
beside the timings is the benchmark's own checksum — a duration alone cannot show
that the faster loop did the same work.

⚠️ **The numbers are this machine, at `-O0`, and they are the demo's point rather
than a promise.** What repeats is the direction, not the digits.

## Build, run, test

```bash
make            # builds the example into the repo-root bin/
make run
make test       # igloo specs: both layouts agree, layout is as claimed, no cold writes
make run-test
```

⚠️ `make test` only **builds** the specs; `make run-test` runs them.
