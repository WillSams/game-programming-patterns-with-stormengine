#pragma once

#include <string>

class Terrain {
public:
  Terrain(int movementCost, bool isWater, bool isGrass, bool isHill,
          int texture);
  ~Terrain();

  int getMovementCost() const;
  bool isWater() const;
  int getTexture() const;
  std::string getTextureName() const;

private:
  int m_movementCost;
  bool m_isHill;
  bool m_isWater;
  bool m_isGrass;
  int m_texture;
};