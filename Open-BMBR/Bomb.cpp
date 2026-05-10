#include "Bomb.hpp"
#include "Map.hpp"
#include <iostream>

// Constructor
Bomb::Bomb(GLfloat bomb_duration, GLfloat y_bomb_position)
    : bomb_duration(bomb_duration), bomb_state(false), fire_active(false)
{
  bomb_position.y = y_bomb_position;
}

void Bomb::setBombPosition(glm::vec3 position) {
  bomb_position.x = (int)std::round(position.x);
  bomb_position.z = (int)std::round(position.z);
}

void Bomb::activateBomb(glm::vec3 position, GLfloat time, Map& map) {
    bomb_state = true;
    bomb_time = time;
    setBombPosition(position);
    MapIndices indices = map.toMapIndices(position);
    map.setBomb(indices);
}

void Bomb::expireBomb(glm::vec3 position, Map& map, GLfloat currentTime) {
  bomb_state = false; 
  MapIndices indices = map.toMapIndices(position);
  map.detonateBomb(indices);

  // Set variables to draw fire
  fire_active = true;
  fire_time = currentTime;
  map.setFire(indices);
}

bool Bomb::isFireActive() {
  return fire_active;
}

void Bomb::putOutFire(MapIndices indices, Map& map) {
  fire_active = false;

  map.extinguishFire(indices);
}

glm::vec3 Bomb::getBombPosition() {
  return bomb_position;
}

GLfloat Bomb::getBombExpiration() {
  return bomb_time + bomb_duration;
}

GLfloat Bomb::getFireExpiration() {
  return fire_time + 1.0f; // 1 sec of fire
}

bool Bomb::getBombState() {
  return bomb_state;
}

// Check if the fire hits a player
bool Bomb::checkCollision(glm::vec3 player_position, const Map& map) {
  MapIndices player_grid_pos = map.toMapIndices(player_position);

  int cell = map.getCell(player_grid_pos.row, player_grid_pos.col);

  if (cell == 6) {
    std::cout << "GAME OVER" << std::endl;
    return true;
  }

  return false;
}
