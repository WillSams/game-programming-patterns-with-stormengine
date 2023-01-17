#include <iostream>
#include <memory>

#include "Game.h"
#include "Unit.h"
#include "commands/MoveUnitCommand.h"

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
  SDL_RenderPresent(m_pRenderer);
}

void Game::update() {}

void Game::handleEvents() {
  Unit unit(0, 0);
  std::unique_ptr<Command> moveCommand =
      std::make_unique<MoveUnitCommand>(unit, 10, 20);
  moveCommand->execute();
  moveCommand->undo();
}

void Game::clean() {
  std::cout << "cleaning game\n";

  SDL_DestroyWindow(m_pWindow);
  SDL_DestroyRenderer(m_pRenderer);
  SDL_Quit();
}
