#include "Bomb.hpp"

// Constructor
Bomb::Bomb(double bomb_duration, double y_bomb_position)
    : bomb_duration(bomb_duration), bomb_state(false)
{
  bomb_position.y = y_bomb_position;
}

void Bomb::setBombPosition(glm::vec3 position) {
  bomb_position.x = (int)std::round(position.x);
  bomb_position.z = (int)std::round(position.z);
}

void Bomb::activateBomb(glm::vec3 position, double time) {
    bomb_state = true;
    bomb_time = time;
    setBombPosition(position);
}

void Bomb::expireBomb() {
  bomb_state = false; 
}

glm::vec3 Bomb::getBombPosition() {
  return bomb_position;
}

double Bomb::getBombExpiration() {
  return bomb_time + bomb_duration;
}

bool Bomb::getBombState() {
  return bomb_state;
}
