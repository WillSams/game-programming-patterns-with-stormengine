#pragma once

#include <SDL2/SDL.h>
#include <string>

enum Direction { LEFT, RIGHT, UP, DOWN };

class Sprite {
public:
  Sprite(SDL_Renderer *&renderer, std::string filePath);
  ~Sprite();

  void draw();
  void move(Direction direction);

private:
  SDL_Texture *m_texture;
  SDL_Rect m_rect;
  SDL_Renderer *m_renderer;
};
