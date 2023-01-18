#include <SDL2/SDL_image.h>
#include <iostream>
#include <memory>

#include "Game.h"
#include "worlds/Terrain.h"
#include "worlds/World.h"

Game::Game() : m_pWindow(), m_pRenderer(), m_Running(false) {}

Game::~Game() { clean(); }

SDL_Renderer *Game::getRenderer() const { return m_pRenderer; }
SDL_Window *Game::getWindow() const { return m_pWindow; }

bool Game::running() { return m_Running; }

void Game::quit() { m_Running = false; }

bool Game::init(const char *title, int xpos, int ypos, int width, int height,
                bool fullscreen) {

  int flags = 0;

  // store the game width and height
  m_gameWidth = width;
  m_gameHeight = height;

  if (fullscreen) {
    flags = SDL_WINDOW_FULLSCREEN;
  }

  // attempt to initialise SDL
  if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
    std::cout << "SDL init success\n";
    // init the window
    m_pWindow = SDL_CreateWindow(title, xpos, ypos, width, height, flags);

    if (m_pWindow != 0) { // window init success
      std::cout << "window creation success\n";
      m_pRenderer = SDL_CreateRenderer(m_pWindow, -1, SDL_RENDERER_ACCELERATED);

      if (m_pRenderer != 0) { // renderer init success
        std::cout << "renderer creation success\n";
        SDL_SetRenderDrawColor(m_pRenderer, 0, 0, 0, 255);
      } else {
        std::cout << "renderer init fail\n";
        return false; // renderer init fail
      }
    } else {
      std::cout << "window init fail\n";
      return false; // window init fail
    }
  } else {
    std::cout << "SDL init fail\n";
    return false; // SDL init fail
  }

  m_Running = true; // everything inited successfully, start the main loop
  return true;
}

void Game::render() {
  SDL_RenderClear(m_pRenderer);

  World *world = new World();
  world->generateTerrain();
  const Terrain &tr = world->getTile(4, 5);

  std::string filename = tr.getTextureName() + ".png";
  SDL_Surface *surface = IMG_Load(filename.c_str());
  SDL_Texture *texture = SDL_CreateTextureFromSurface(m_pRenderer, surface);
  SDL_FreeSurface(surface);

  SDL_Rect src;
  src.x = 0;
  src.y = 0;
  src.w = 32;
  src.h = 32;

  SDL_Rect dest;
  dest.x = 0;
  dest.y = 0;
  dest.w = 32;
  dest.h = 32;

  SDL_RenderCopy(m_pRenderer, texture, &src, &dest);
  SDL_RenderPresent(m_pRenderer);
}

void Game::update() {}

void Game::handleEvents() {}

void Game::clean() {
  std::cout << "cleaning game\n";

  SDL_DestroyWindow(m_pWindow);
  SDL_DestroyRenderer(m_pRenderer);
  SDL_Quit();
}
