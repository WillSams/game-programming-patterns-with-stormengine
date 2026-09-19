# Shared specs

Not a pattern. This folder exists so that headers **shared by every pattern** have
somewhere to be spec'd.

`include/pixelFont.h` — the 3x5 font table, and the coverage spec that goes with it
— was copied into two patterns (byte-identical: the table, its SDL drawer and its
spec) with eight patterns still to write. `CODING.md` tenet 1 says the shared
*decision* belongs in one place, so the header moved to `include/` and its spec
came here.

Why a folder: the per-pattern `Makefile` only globs `specs/*.cpp`, so a shared spec
had no home, and the CI loop runs "any folder with a Makefile". The `main.cpp` in
here is a stub purely so that loop's `make` step has something to build.

```bash
make test && make run-test
```
