#ifndef BOMB_HPP
#define BOMB_HPP

#include <glm/glm.hpp>
#include <GL/glew.h>
#include "Map.hpp"

class Bomb {
public:
  // Constructor
  Bomb (GLfloat bomb_duration, GLfloat y_bomb_position);

  void setBombPosition(glm::vec3 position);
  void activateBomb(glm::vec3 position, GLfloat time, Map& map, int radius);
  void expireBomb(glm::vec3 position, Map& map, GLfloat currentTime);

  bool isFireActive();
  void putOutFire(MapIndices indices, Map& map);
  glm::vec3 getBombPosition();
  GLfloat getBombExpiration();
  GLfloat getFireExpiration();
  bool getBombState();
  bool checkCollision(glm::vec3 player_position, const Map& map);
  int getBlastRadius();

private:
  // Bomb Atributes
  glm::vec3 bomb_position;
  GLfloat bomb_time;
  GLfloat bomb_duration;
  bool bomb_state;
  bool fire_active;
  GLfloat fire_time;
  int blastRadius = 1;
};

#endif
