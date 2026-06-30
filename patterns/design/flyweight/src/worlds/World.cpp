#include "World.h"

#include <random>

World::World()
    : m_grassTerrain(1, false, true, false, 0),
      m_riverTerrain(2, true, false, false, 1),
      m_hillTerrain(3, false, false, true, 2) {}

// No destructor: m_tiles holds non-owning pointers into the three member
// terrains, so there is nothing to delete (the old version double-freed
// member objects).

void World::generateTerrain(unsigned seed) {
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> dis(0, 2);

    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            switch (dis(gen)) {
            case 0:  m_tiles[x][y] = &m_grassTerrain; break;
            case 1:  m_tiles[x][y] = &m_riverTerrain; break;
            default: m_tiles[x][y] = &m_hillTerrain;  break;
            }
        }
    }
}

const Terrain &World::getTile(int x, int y) const { return *m_tiles[x][y]; }
