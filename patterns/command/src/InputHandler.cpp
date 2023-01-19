#include <functional>
#include <memory>
#include <string>

#include "InputHandler.h"

#include "commands/JumpCommand.h"
#include "commands/MoveLeftCommand.h"
#include "commands/MoveRightCommand.h"

InputHandler::InputHandler() : m_key_map() {
  // Bind keys to commands
  m_key_map[SDL_SCANCODE_LEFT] = new MoveLeftCommand();
  m_key_map[SDL_SCANCODE_RIGHT] = new MoveRightCommand();
  m_key_map[SDL_SCANCODE_SPACE] = new JumpCommand();
}

Command *InputHandler::handleInput() {
  // Handle input
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_KEYDOWN) {
      auto it = m_key_map.find(event.key.keysym.scancode);
      if (it != m_key_map.end()) {
        return it->second;
      }
    }
  }
  return nullptr;
}
