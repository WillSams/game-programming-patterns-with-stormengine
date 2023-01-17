#pragma once

#include <iostream>
#include <vector>

#include <SDL2/SDL.h>

class Game {
private:
  SDL_Window *m_pWindow;
  SDL_Renderer *m_pRenderer;

  bool m_Running;

  int m_gameWidth;
  int m_gameHeight;

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
};
