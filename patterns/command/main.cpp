#include <iostream>
#include <memory>

#include "src/Game.h"

const int FPS = 60;
const unsigned int DELAY_TIME = 1000.0f / FPS;

int main(int argc, char **argv) {
  Uint32 frameStart, frameTime;

  auto game = std::make_unique<Game>();

  std::cout << "game init attempt...\n";
  if (game->init("Command Pattern Example", 100, 100, 640, 480, false)) {
    std::cout << "game init success!\n";
    while (game->running()) {
      frameStart = SDL_GetTicks();

      game->handleEvents();
      game->update();
      game->render();

      frameTime = SDL_GetTicks() - frameStart;

      if (frameTime < DELAY_TIME) {
        SDL_Delay((int)(DELAY_TIME - frameTime));
      }
    }
  } else {
    std::cout << "game init failure - " << SDL_GetError() << "\n";
    return -1;
  }

  std::cout << "game closing...\n";
  game->clean();

  return 0;
}