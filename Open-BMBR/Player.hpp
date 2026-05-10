#ifndef PLAYER_HPP
#define PLAYER_HPP
#define GLM_ENABLE_EXPERIMENTAL

#include "Map.hpp"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

enum Player_Movement { NORTH, SOUTH, WEST, EAST };

class Player {
public:
  // Constructor
  Player(glm::vec3 position, glm::vec2 facing);

  glm::vec3 getPosition();
  glm::quat getOrientation();
  void setCanPassBomb(bool value);
  void ProcessKeyboard(Player_Movement direction, GLfloat deltaTime,
                       const Map &map);
  void ProcessKeyboardFPS(Player_Movement direction, glm::vec3 cameraFront,
                          glm::vec3 cameraRight, GLfloat deltaTime,
                          const Map &map);
  bool CheckCollision(glm::vec3 newPos, const Map &map);

private:
  // Player Attibutes
  glm::vec3 position;
  glm::quat orientation;
  GLfloat speed;
  float halfHitbox = 0.3f;
  bool canPassBomb;
};

#endif
