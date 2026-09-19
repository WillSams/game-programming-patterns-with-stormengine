# Event Queue Pattern

> Reference: [Game Programming Patterns — Event Queue](https://gameprogrammingpatterns.com/event-queue.html)

**The sender does not know who listens.** Without this, the combat code has to call
the audio code, the achievement code and the UI code — and the more it announces,
the more it knows, until nothing can be added or removed without touching it. A
queue breaks that: the sender puts an event in and knows nothing more; whoever cares
takes it out.

The demo shows the pattern as **two columns of the same moment** — what the *sender*
put in, and what the *listeners* did about it — produced by different code that
never mentions each other.

## How it works

- `Event` and `EventType` (`src/events/eventQueue.h`) are the vocabulary, with one
  `DescribeEvent` used by both the specs and the screen so they cannot disagree.
- `EventQueue` is a fixed-capacity ring buffer with `send`, `next` and `dispatch`.
- `PlayState` sends events on keypress and wires the handlers. **The handlers are
  nowhere near the sender** — that separation is the pattern, and it is why the two
  columns are on screen.

## Controls

| Key | Action |
| --- | --- |
| S | Send a `Scored` event — and watch the listeners answer it |
| D | Send a `TookDamage` event |
| J | Send a `Jumped` event |
| 1 / 2 | Switch the sound / achievement listener on and off |
| R | Clear both columns |
| Esc | Quit |

Press **S** and read both columns: the sender contributed one line, and the
listeners produced three (`SOUND` → `ACHIEVEMENT`) with `STATUS DRAINED 3`. Switch a
listener off with 1 or 2 and press S again — **the sender is not told, and does not
need to be.**

## Build, run, test

```bash
make            # builds the example into the repo-root bin/
make run
make test       # igloo specs for order, capacity, and dispatch-time sends
make run-test
```

⚠️ `make test` only **builds** the specs; `make run-test` runs them.
