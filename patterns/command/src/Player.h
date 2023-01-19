
#include "Sprite.h"
#include "commands/Command.h"

class Player {
public:
  Player(Sprite sprite);
  void render();
  void update(Command *command);

private:
  Sprite m_sprite;
};
