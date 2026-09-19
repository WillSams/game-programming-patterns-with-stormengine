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

## Controls

| Key | Action |
| --- | --- |
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
