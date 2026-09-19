# Object Pool Pattern

> Reference: [Game Programming Patterns — Object Pool](https://gameprogrammingpatterns.com/object-pool.html)

**Allocate once, then hand out what is free.** A fountain of particles that are
created and destroyed every frame is a `new`/`delete` pair per particle and a
fragment the heap keeps for the rest of the run. A pool builds the whole set up
front and recycles it, so the work is paid before the first frame instead of
during every one.

## How it works

- `ParticlePool` (`src/pool/particlePool.h`) owns **one fixed vector of slots**,
  sized at construction and never resized — not when it fills, not on `Clear`.
- `Create` takes the head of the free list and returns its **index**, or `-1` when
  the pool is full. `Release` pushes the slot back. Both are O(1).
- **The free list lives inside the objects.** A free slot stores the index of the
  next free slot in its own `nextFree`, so the pool needs no side array and no
  second allocation. That is the trick the chapter spends its sample code on.
- `Animate` walks **every** slot, live or dead, and skips the dead with one branch.
  A pool does not get cheaper as it empties; it gets predictable.

### ⚠️ Two things the chapter leaves to the reader, and this one does not

**Exhaustion is refused, never overwritten.** A full pool returns `-1` and the
caller decides what to do. The tempting alternative — recycle the oldest object to
make room — looks like it works, because bursts keep appearing, while it silently
cuts the life of something already on screen. `REFUSED` counts the refusals, and a
spec proves the refused spawn left the live particle's every field untouched.

**Reuse is invisible, and that is the pattern's real danger.** A returned object
still has its old bytes at the same address, so a slot that is read without asking
`IsAlive` first yields a **plausible ghost** — the values look like a particle. The
specs pin it: after a slot is released and taken again, what comes back is the new
particle's data and nothing of the old one.

⚠️ This implementation hands out an **index** and requires `IsAlive` before `At`.
It does **not** hand out generation counters, which would catch a handle held
across a frame — a demo that never holds one would carry the machinery unused.

## Controls

| Key | Action |
|---|---|
| Space | Spawn one particle |
| B | Burst of 20 — press it faster than particles expire |
| A | Auto-fountain on/off |
| R | Clear the counters (the pool is untouched) |
| 0 | Empty the pool — capacity is unchanged |
| Esc | Quit |

**Read the right column.** Leave auto on and `CREATED` climbs forever while
`CAPACITY` stays at 120 and `REUSED` tracks it — the fountain on screen has been
handed out hundreds of times and allocated once. When `REFUSED` starts climbing the
pool is full, and the fountain does **not** grow to accommodate you; that is the
fixed commitment, not a bug.

The **strip under the field** is every slot, alive in amber and free in dark. It is
the pool as a shape: the row never grows, and the amber cells move around inside it.

## Build, run, test

```bash
make            # builds the example into the repo-root bin/
make run
make test       # igloo specs: fixed capacity, refusal, reuse, free-list integrity
make run-test
```

⚠️ `make test` only **builds** the specs; `make run-test` runs them.