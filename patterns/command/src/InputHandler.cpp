#include <SDL2/SDL.h>
#include <memory>

#include "InputHandler.h"

#include "Game.h"
#include "commands/Command.h"
#include "commands/JumpCommand.h"
#include "commands/MoveCommand.h"
#include "commands/MoveLeftCommand.h"
#include "commands/MoveRightCommand.h"

InputHandler::InputHandler()
    : m_keystates(SDL_GetKeyboardState(nullptr)), m_lastCommand(nullptr) {
  bindKey(SDLK_LEFT, std::make_unique<MoveLeftCommand>());
  bindKey(SDLK_RIGHT, std::make_unique<MoveRightCommand>());
  bindKey(SDLK_SPACE, std::make_unique<JumpCommand>());
}

void InputHandler::handleInput(SDL_Event &event) {
  if (event.type == SDL_KEYDOWN) {
    auto it = m_key_map.find(event.key.keysym.sym);
    if (it != m_key_map.end()) {
      m_lastCommand = it->second.get();
      it->second->getDirection();
    }
  } else if (event.type == SDL_KEYUP) {
    m_lastCommand = nullptr;
  }
}

void InputHandler::bindKey(SDL_Keycode key, std::unique_ptr<Command> command) {
  m_key_map[key] = std::move(command);
}

Command *InputHandler::getLastCommand() { return m_lastCommand; }
