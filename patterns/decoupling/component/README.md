# Component Pattern

> Reference: [Game Programming Patterns — Component](https://gameprogrammingpatterns.com/component.html)

**An entity is a container, not a class in a hierarchy.** One class per kind of
thing does not scale: a player needs input and physics and drawing, a projectile
needs physics and drawing but no input, a decoration needs drawing alone.
Inheritance answers that with a base class and a subclass per combination, or with
one bloated base holding flags. Composition answers it with parts.

So there is no `Player` class and no `Projectile` class. There is `Entity`, and what
each one **is** is what it is **made of**.

## How it works

- `Component` (`src/components/component.h`) is the seam, and it is small on
  purpose: a component may change the entity's `Transform`, read the `InputState`,
  and append a `DrawItem`. It cannot reach the world, the renderer or another
  entity — an entity is handed to it and nothing else. That is the chapter's
  "components should not know about each other" as a fact rather than as advice.
- `components.h` holds the three parts — `InputComponent`, `PhysicsComponent`,
  `AppearanceComponent` — and every entity in the demo is a different combination.
- ⚠️ **The pure layer DESCRIBES, it does not draw.** An appearance component appends
  a `DrawItem` (a position and a size, plain floats); `PlayState` is the only place
  SDL appears. That keeps the pattern spec-able with no window, and it is what the
  chapter's `GraphicsComponent` is really doing.
- Each key advances **one tick**, so the composition is visible: press RIGHT and the
  input component sets a velocity, the physics component moves the entity, and the
  appearance component describes it where it now is.

## Controls

| Key | Action |
| --- | --- |
| A / D (or Left / Right) | One tick, moving left / right |
| Space (or Up) | One tick, jumping — only from the ground |
| T | Ten ticks with no input, so the projectile's fall is visible |
| R | Reset every entity |
| 1 – 3 | Inspect the player / projectile / prop |
| Esc | Quit |

## Build, run, test

```bash
make            # builds the example into the repo-root bin/
make run
make test       # igloo specs for the parts, the order, and the composition
make run-test
```

⚠️ `make test` only **builds** the specs; `make run-test` runs them.
