#pragma once

#include <random>

#include "Terrain.h"

const int WIDTH = 10;
const int HEIGHT = 10;

class World {
public:
  World();
  ~World();

  void generateTerrain();
  const Terrain &getTile(int x, int y) const;

private:
  Terrain m_grassTerrain;
  Terrain m_riverTerrain;
  Terrain m_hillTerrain;
  Terrain *m_tiles[WIDTH][HEIGHT];
};