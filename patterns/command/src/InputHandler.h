#include <SDL2/SDL.h>
#include <unordered_map>
#include <vector>

#include "commands/Command.h"

class InputHandler {
public:
  InputHandler();
  Command *handleInput();

private:
  std::unordered_map<SDL_Scancode, Command *> m_key_map;
};