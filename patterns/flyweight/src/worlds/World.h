#pragma once

#include "Terrain.h"

const int WIDTH  = 10;
const int HEIGHT = 10;

// The flyweight in action: a WIDTH x HEIGHT grid of tiles, but every tile just
// points at one of three shared Terrain instances. A 10x10 world costs three
// Terrain objects, not one hundred.
class World {
public:
    World();

    // Fills the grid with shared terrain. Seeded so the layout is reproducible
    // (and testable).
    void generateTerrain(unsigned seed);

    const Terrain &getTile(int x, int y) const;

private:
    // The shared (intrinsic) terrain types — owned once, referenced everywhere.
    Terrain m_grassTerrain;
    Terrain m_riverTerrain;
    Terrain m_hillTerrain;

    // Non-owning pointers into the three terrains above.
    const Terrain *m_tiles[WIDTH][HEIGHT] = {};
};
