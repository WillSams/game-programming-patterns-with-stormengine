# Type Object Pattern

> Reference: [Game Programming Patterns — Type Object](https://gameprogrammingpatterns.com/type-object.html)

**One class for every kind of thing, plus an object holding what varies.** There is
no `Dragon` and no `Brute` — there is `Monster`, and a `Breed` it points at. A new
kind of creature is a new **value**, not a new class: no header, no subclass, no
rebuild.

The chapter's two payoffs are both in the code:

1. **A breed can be based on another breed.** Each breed states only what it
   *changes* and inherits the rest, so `DRAGON` has 100 health and attack 12 with
   `attack` never written twice — and `attack` comes from its **grandparent**.
2. **Many things share one type.** A hundred monsters point at one `Breed`, which
   is what makes this cheaper than a class per kind.

## How it works

- `Breed` (`src/types/breed.h`) is the type object: a name, health, attack, and a
  `parent` to inherit from. `healthValue()` / `attackValue()` walk the chain, and
  that walk **is** the inheritance.
- `Monster` (`src/types/monster.h`) is the one class. It owns its **identity and
  its current health**; everything else is the breed's.
- `bestiary.h` holds the breeds as data.
- `PlayState` is presentation: the breed list whose rows show *answered* values, the
  spawned monsters, and a panel that reports how many type objects they use.

## Controls

| Key | Action |
| --- | --- |
| 1 – 3 | Select a breed |
| Space | Spawn a monster of the selected breed |
| D / H | Damage / heal **every** spawned monster |
| R | Remove them all |
| Esc | Quit |

Spawn four brutes and watch the SHARED TYPE panel: **4 MONSTERS / 1 BREED IN USE /
ALL SHARE ONE**. That sentence is computed from the monsters rather than asserted —
the panel counts the distinct breeds they actually point at, by address, which is
the same equality the specs pin.

## Build, run, test

```bash
make            # builds the example into the repo-root bin/
make run
make test       # igloo specs for the breed walk, sharing, and combat
make run-test
```

⚠️ `make test` only **builds** the specs; `make run-test` runs them.
