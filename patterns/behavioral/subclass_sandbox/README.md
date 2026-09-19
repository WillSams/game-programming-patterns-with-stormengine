# Subclass Sandbox Pattern

> Reference: [Game Programming Patterns — Subclass Sandbox](https://gameprogrammingpatterns.com/subclass-sandbox.html)

A base class provides a set of operations — a **sandbox** — and every subclass
implements one method by composing them. The base decides what a subclass is
*allowed to say*; the subclass decides what to say with it.

Two payoffs, and the chapter is explicit that both matter:

1. **The vocabulary is deliberate.** A power can `move`, `playSound`,
   `spawnParticles`, `addHealth`, `setHealth` — and nothing else. It cannot reach
   the world, the renderer or the audio system, because it is never handed them.
   Subclasses are powerful **and contained**.
2. **Shared work is hoisted, not copied.** `jumpTo` is built *from* the
   vocabulary, so every power that jumps jumps the same way. Containment bought
   at the price of duplication would be a bad trade.

## How it works

- `Sandbox` (`src/powers/sandbox.h`) is the world a power may act on, and **every
  operation is recorded**. That is what makes the pattern testable without a
  window: a power's effect is a list, and the specs assert the list.
- `Superpower` (`src/powers/superpower.h`) is the base: the protected vocabulary,
  the shared `jumpTo`, and one public entry point.
- `powers.h` holds the four powers. Each is a **name and a few lines** — no member
  state, no constructor, no resource of its own.
- `PlayState` is presentation: the power list, the effect log, and a box showing
  where the power moved the world.

### `activate` is public and NON-virtual, and that is the pattern made structural

```cpp
void activate(Sandbox &world);     // one entry point, not overridable
protected:
virtual void onActivate() = 0;     // the subclass's entire job
```

`onActivate` has **no world parameter at all**, so the only route to the sandbox
is the protected vocabulary in the base. A public `virtual activate(Sandbox&)`
would have let any subclass ignore the sandbox and take whatever it liked — the
constraint would have been a convention instead of a fact. (The idiom is
Non-Virtual Interface.)

## Controls

| Key | Action |
|---|---|
| 1 – 4 | Activate a power (Sky Launch · Ground Dive · Super Jump · Fireball) |
| R | Reset the world and clear the log |
| Esc | Quit |

The screen resets the sandbox before each activation so the EFFECTS panel reads as
"what this power did". **The pattern itself accumulates happily** — the specs pin
that — but a list that grew longer with every keypress would teach the wrong thing.

## Build, run, test

```bash
make            # builds the example into the repo-root bin/
make run
make test       # igloo specs for the vocabulary and the powers
make run-test
```

⚠️ `make test` only **builds** the specs; `make run-test` runs them.
