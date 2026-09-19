# Service Locator Pattern

> Reference: [Game Programming Patterns — Service Locator](https://gameprogrammingpatterns.com/service-locator.html)

**The game asks for the service it needs without naming the class that provides
it**, and without that provider being threaded through every constructor between
here and the code that actually wants it.

⚠️ **THE CHAPTER IS EXPLICITLY UNSURE ABOUT THIS ONE, AND SO IS THIS README.** The
book calls it "one of the most misunderstood" of its patterns: the locator is a
**global**, so it *hides* dependencies rather than declaring them — a class that
calls `ServiceLocator::Audio()` looks like it needs nothing, and the only way to
learn otherwise is to read it. The chapter recommends **dependency injection** where
that choice exists, and this repo takes that seriously: the other demos here pass
their dependencies as constructor arguments.

The honest reading is that it is what you reach for when a global really is what you
have (an audio device, a platform service) and threading it through forty
constructors would be worse — **and even then, the null service and a way to
unregister are what keep it from becoming a hazard.** Both are pinned by spec here
rather than left as advice.

## How it works

Three parts, one file each:

| Part | Where |
|---|---|
| **The service** — the interface the game talks to | `src/services/service.h` |
| **The null service** — "do nothing", so no caller checks for null | same file |
| **The locator** — the global access point | `src/services/serviceLocator.h` |
| **The provider** — what actually answers | the demo's `OnScreenAudioService`, and the specs' test double |

### The four things this implementation promises, each pinned by a spec

1. **`Audio()` never returns null.** With nothing registered it returns the null
   service, which is what replaces `if (audio)` at every call site.
2. **`Provide(nullptr)` unregisters** — the book's own idiom, and the path that
   lets a test put the world back.
3. **The locator does not own what it is handed.** It stores a reference; the caller
   keeps the provider alive. A spec resets the locator and then uses the provider
   again, because a locator that deleted its provider would be a lifetime trap
   nobody signed up for.
4. **The same caller behaves identically under any provider.** A spec runs one caller
   against two providers and asserts the caller's *own* log is unchanged — the
   decoupling claim, tested rather than asserted.

And two states of the global, stated: a **swap is not synchronised** (a data race
if another thread is mid-call — this is a single-threaded seam), and **nothing here
is safe after `main` returns**.

## Controls

| Key | Action |
|---|---|
| P | Call `PlaySound("goal")` |
| M | Call `PlayMusic("arena")` |
| X | Call `StopAll()` |
| 1 | Register the console audio provider |
| 2 | Register the **null** provider |
| R | Clear both panels |
| Esc | Quit |

**The demo is the payoff in two counters.** Press P and M a few times and read
`CALLS 4  HEARD 4`. Now press **2** and do it again: the game keeps calling —
`CALLS 8` — and `HEARD` **stays at 4**, while the service panel reads `NOTHING YET`.
The caller did not change, and was not told. That is the pattern, and it is also
why the null service exists: without it, the code that does not want to be told has
to keep saying so at every call site.

## Build, run, test

```bash
make            # builds the example into the repo-root bin/
make run
make test       # igloo specs: the null service, swapping, ownership, and the global
make run-test
```

⚠️ `make test` only **builds** the specs; `make run-test` runs them.
