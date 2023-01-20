#pragma once

#include <SDL2/SDL.h>

#include <SDL2/SDL.h>
#include <map>

#include "Player.h"
#include "commands/Command.h"

class InputHandler {
public:
  InputHandler();
  void handleInput(SDL_Event &event);
  void bindKey(SDL_Keycode key, std::unique_ptr<Command> command);
  Command *getLastCommand();

private:
  const Uint8 *m_keystates;
  std::map<SDL_Keycode, std::unique_ptr<Command>> m_key_map;
  Command *m_lastCommand;
};