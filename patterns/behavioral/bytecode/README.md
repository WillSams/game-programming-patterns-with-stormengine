# Bytecode Pattern

> Reference: [Game Programming Patterns — Bytecode](https://gameprogrammingpatterns.com/bytecode.html)

**Behaviour as data, executed by a machine that knows nothing about the
behaviour.** A spell is not a class — it is a `Program`: a list of instructions.
The machine has a program counter, a stack and a little game state; the *content*
lives outside it, in data that could have been loaded from a file.

The pattern pays off when the interesting part of your game **is** data but the
ways of combining it are code. Here the combinations are the **opcodes** and the
spells are the content, so adding a spell is adding a value — no new class, no new
opcode, no rebuild of the machine.

## How it works

- `Instruction` and `OpCode` (`src/vm/instruction.h`) are the instruction set, as
  data. ⚠️ An instruction **may** carry an operand and most do not: `PushLiteral`
  needs a value, `HalveHealth` needs nothing. One struct with an operand that some
  opcodes ignore is simpler than two instruction types — and the alternative the
  chapter warns against, a separate `Literal` instruction that mutates the *next*
  instruction, makes a program unreadable and unseekable.
- `BytecodeVm` (`src/vm/bytecodeVm.h`) is the machine: `step()` runs one
  instruction, `run()` runs to a halt, and `StepResult` says which of
  **Advanced / Halted / Faulted** happened. Pure C++, no SDL, no engine — the specs
  reach it directly.
- `spells.h` holds the programs, including one that is **deliberately wrong**.
- `PlayState` only draws: the listing with the program counter on it, the machine's
  state, and its stack.

## Controls

| Key | Action |
| --- | --- |
| Space | Run **one** instruction, so the machine can be watched |
| Enter | Run to a halt (or a fault) |
| R | Rewind to the start of the same program |
| 1 – 4 | Pick a spell (the fourth one faults, on purpose) |
| Esc | Quit |

## Build, run, test

```bash
make            # builds the example into the repo-root bin/
make run
make test       # igloo specs for the machine and the font
make run-test
```

⚠️ `make test` only **builds** the specs; `make run-test` runs them.
