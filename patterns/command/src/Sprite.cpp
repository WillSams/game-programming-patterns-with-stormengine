#include <SDL2/SDL_image.h>

#include "Sprite.h"

Sprite::Sprite(std::string filePath)
    : m_rect({0, 0, 0, 0}), m_texture(nullptr), {
  // Load image into texture
  SDL_Surface *surface = IMG_Load(filePath.c_str());
  m_texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);
  // Set rect to the size of the image
  SDL_QueryTexture(m_texture, NULL, NULL, &m_rect.w, &m_rect.h);
}

void Sprite::draw() { SDL_RenderCopy(renderer, m_texture, NULL, &m_rect); }

void Sprite::move(Direction direction) {
  switch (direction) {
  case Direction::LEFT:
    m_rect.x -= 10;
    break;
  case Direction::RIGHT:
    m_rect.x += 10;
    break;
  }
}
