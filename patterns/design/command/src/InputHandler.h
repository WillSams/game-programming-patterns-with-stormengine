#pragma once

#include <SDL2/SDL.h>
#include <map>
#include <memory>

#include "commands/Command.h"

// Binds keys to Command objects and reports the command currently being issued.
// Rebinding a key is just swapping its command — the rest of the game is
// unaware of which key does what.
class InputHandler {
public:
  InputHandler();

  void handleInput(const SDL_Event &event);
  void bindKey(SDL_Keycode key, std::unique_ptr<Command> command);
  Command *getLastCommand() const;

private:
  std::map<SDL_Keycode, std::unique_ptr<Command>> m_keyMap;
  Command *m_lastCommand = nullptr;
};
