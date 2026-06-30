#include "InputHandler.h"

#include "commands/JumpCommand.h"
#include "commands/MoveLeftCommand.h"
#include "commands/MoveRightCommand.h"

InputHandler::InputHandler() {
  bindKey(SDLK_LEFT,  std::make_unique<MoveLeftCommand>());
  bindKey(SDLK_RIGHT, std::make_unique<MoveRightCommand>());
  bindKey(SDLK_UP,    std::make_unique<JumpCommand>());
  bindKey(SDLK_SPACE, std::make_unique<JumpCommand>());
}

void InputHandler::handleInput(const SDL_Event &event) {
  if (event.type == SDL_KEYDOWN) {
    auto it = m_keyMap.find(event.key.keysym.sym);
    m_lastCommand = (it != m_keyMap.end()) ? it->second.get() : nullptr;
  } else if (event.type == SDL_KEYUP) {
    m_lastCommand = nullptr;
  }
}

void InputHandler::bindKey(SDL_Keycode key, std::unique_ptr<Command> command) {
  m_keyMap[key] = std::move(command);
}

Command *InputHandler::getLastCommand() const { return m_lastCommand; }
