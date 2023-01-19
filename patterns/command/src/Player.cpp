#include "Player.h"
#include "commands/Command.h"
#include "commands/MoveCommand.h"

Player::Player(Sprite sprite) : m_sprite(sprite) {}

void Player::render() { m_sprite.draw(); }

void Player::update(Command *command) {
  if (auto moveCommand = dynamic_cast<MoveCommand *>(command)) {
    m_sprite.move(moveCommand->getDirection());
  }
}