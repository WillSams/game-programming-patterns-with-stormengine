#include <random>

#include "World.h"

World::World()
    : m_grassTerrain(1, false, true, false, 0),
      m_riverTerrain(2, true, false, false, 1),
      m_hillTerrain(3, false, false, true, 2), m_tiles() {}

World::~World() {
  for (int i = 0; i < WIDTH; i++) {
    for (int j = 0; j < HEIGHT; j++) {
      delete m_tiles[i][j];
    }
  }
}

void World::generateTerrain() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, 2);

  for (int i = 0; i < WIDTH; i++) {
    for (int j = 0; j < HEIGHT; j++) {
      int terrainType = dis(gen);
      switch (terrainType) {
      case 0:
        m_tiles[i][j] = &m_grassTerrain;
        break;
      case 1:
        m_tiles[i][j] = &m_riverTerrain;
        break;
      case 2:
        m_tiles[i][j] = &m_hillTerrain;
        break;
      }
    }
  }
}

const Terrain &World::getTile(int x, int y) const { return *m_tiles[x][y]; }
