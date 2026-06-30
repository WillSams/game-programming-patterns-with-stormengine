# Command Pattern

The command pattern encapsulates input as command objects, decoupling *what*
triggers an action (a key) from the action itself. This makes input remappable
and keeps game logic unaware of specific keys.

Built on **Storm Engine v2**: `Game` runs the loop via a `GameStateMachine`, and
the demo lives in `PlayState`.

## How it works

- `InputHandler` binds keys to `Command` objects (`bindKey`), so rebinding is
  just swapping a command.
- `PlayState` feeds SDL events to the `InputHandler` and each frame executes the
  current command on the `Player` — it never references a specific key or action.
- Concrete commands (`MoveLeftCommand`, `MoveRightCommand`, `JumpCommand`)
  resolve to a `Direction`; the `Player` moves accordingly.

## Controls

| Key | Command |
|---|---|
| Left arrow | Move left |
| Right arrow | Move right |
| Up arrow / Space | Jump (up) |
| Esc | Quit |

## Build, run, test

```bash
make            # builds ../../bin/command_pattern_example
make run
make test       # igloo specs for the command classes
make run-test
```
