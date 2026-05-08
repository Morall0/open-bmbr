#ifndef BOMB_HPP
#define BOMB_HPP

#include <glm/glm.hpp>
// #include <GL/glew.h>
#include "Map.hpp"

class Bomb {
public:
  // Constructor
  Bomb (double bomb_duration, double y_bomb_position);

  void setBombPosition(glm::vec3 position);
  void activateBomb(glm::vec3 position, double time, Map& map);
  void expireBomb(glm::vec3 position, Map& map);

  glm::vec3 getBombPosition();
  double getBombExpiration();
  bool getBombState();

private:
  // Bomb Atributes
  glm::vec3 bomb_position;
  double bomb_time;
  double bomb_duration;
  bool bomb_state;
};

#endif
