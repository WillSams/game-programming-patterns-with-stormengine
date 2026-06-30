#include "Terrain.h"

Terrain::Terrain(int movementCost, bool isWater, bool isGrass, bool isHill,
                 int texture)
    : m_movementCost(movementCost), m_isWater(isWater), m_isGrass(isGrass),
      m_isHill(isHill), m_texture(texture) {}

Terrain::~Terrain() {}

int Terrain::getMovementCost() const { return m_movementCost; }
bool Terrain::isWater() const { return m_isWater; }
int Terrain::getTexture() const { return m_texture; }
std::string Terrain::getTextureName() const {
  switch (m_texture) {
  case 0:
    return "grass";
  case 1:
    return "water";
  case 2:
    return "hill";
  }
  return "Unknown";
}
