#include <gtest/gtest.h>

#include "../src/Game.h"

std::unique_ptr<Game> game;

void SetUp() { game = std::make_unique<Game>(); }

TEST(Game, running) {
  auto game = std::make_unique<Game>();
  EXPECT_EQ(game->running(), false);
}

// TEST(Game, init) { EXPECT_EQ(game->init("test", 0, 0, 0, 0, false), true); }

// TEST(Game, getRenderer) { EXPECT_EQ(game->getRenderer(), nullptr); }

// TEST(Game, getWindow) { EXPECT_EQ(game->getWindow(), nullptr); }

// TEST(Game, quit) {
//   game->quit();
// }

// TEST(Game, render) {
//   game->render();
// }

// TEST(Game, update) {
//   game->update();
// }

// TEST(Game, handleEvents) {
//   game->handleEvents();
// }

// TEST(Game, clean) {
//   game->clean();
// }
