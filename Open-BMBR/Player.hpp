#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <glm/glm.hpp>
#include <GL/glew.h>
#include "Map.hpp"

enum Player_Movement
{
	NORTH,
	SOUTH,
	WEST,
	EAST
};

class Player {
public:
  // Constructor
  Player (glm::vec3 position, glm::vec2 direction);

  glm::vec3 getPosition();
  glm::vec2 getDirection();
  void ProcessKeyboard(Player_Movement direction, GLfloat deltaTime, const Map& map);
  bool CheckCollision(glm::vec3 newPos, const Map& map);

private:
  // Player Attibutes
  glm::vec3 position;
  glm::vec3 direction;
  GLfloat speed;
  float halfHitbox = 0.3f;
};

#endif
