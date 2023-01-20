#include <SDL2/SDL_image.h>

#include <iostream>

#include "Sprite.h"

Sprite::Sprite(SDL_Renderer &renderer, std::string filePath)
    : m_rect({0, 0, 0, 0}), m_texture(nullptr), m_renderer(renderer) {
  // Load image into texture
  SDL_Surface *surface = IMG_Load(filePath.c_str());
  m_texture = SDL_CreateTextureFromSurface(&renderer, surface);
  SDL_FreeSurface(surface);

  // Set rect to the size of the image
  SDL_QueryTexture(m_texture, NULL, NULL, &m_rect.w, &m_rect.h);
}

Sprite::~Sprite() { SDL_DestroyTexture(m_texture); }

void Sprite::draw() { SDL_RenderCopy(&m_renderer, m_texture, NULL, &m_rect); }

void Sprite::move(Direction direction) {
  switch (direction) {
  case Direction::LEFT:
    std::cout << "Moving left"
              << "\n";
    m_rect.x -= 10;
    break;
  case Direction::RIGHT:
    std::cout << "Moving right"
              << "\n";
    m_rect.x += 10;
    break;
  case Direction::UP:
    std::cout << "Moving up"
              << "\n";
    m_rect.y -= 10;
    break;
  case Direction::DOWN:
    std::cout << "Moving down"
              << "\n";
    m_rect.y += 10;
    break;
  }
}
