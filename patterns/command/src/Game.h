#pragma once

#include <SDL2/SDL.h>

#include <iostream>
#include <vector>

#include "InputHandler.h"
#include "Player.h"

class Game {

public:
  Game();
  ~Game();

  SDL_Renderer *getRenderer() const;
  SDL_Window *getWindow() const;

  bool running();

  void quit();

  bool init(const char *title, int xpos, int ypos, int width, int height,
            bool fullscreen);

  void render();
  void update();
  void handleEvents();
  void clean();

private:
  SDL_Window *m_pWindow;
  SDL_Renderer *m_pRenderer;
  std::unique_ptr<Player> m_pPlayer;
  std::unique_ptr<InputHandler> m_pInputHandler;

  bool m_Running;

  int m_gameWidth;
  int m_gameHeight;
};
