#include "Player.hpp"
#include "Map.hpp"
#include <cmath>
#include <glm/geometric.hpp>

const GLfloat SPEED = 4.0f;

// Constructor
Player::Player(glm::vec3 position, glm::vec2 facing)
    : position(position), speed(SPEED)
{
    glm::vec3 dir = glm::normalize(glm::vec3(facing.x, 0.0f, facing.y));

    // Quaternion -Z (forward base) to initial dir
    orientation = glm::rotation(glm::vec3(0,0,1), dir);
}

glm::vec3 Player::getPosition()
{
  return this->position;
}

glm::quat Player::getOrientation()
{
  return orientation;
}

void Player::ProcessKeyboard(Player_Movement direction, GLfloat deltaTime, const Map& map)
{
  GLfloat velocity = this->speed * deltaTime;
  glm::vec3 move(0.0f);

  if (direction == NORTH)
    move.z -= 1.0f;

  if (direction == SOUTH)
    move.z += 1.0f;

  if (direction == EAST)
    move.x += 1.0f;

  if (direction == WEST)
    move.x -= 1.0f;

  // Progressive rotation
  if (glm::length(move) > 0.0f)
  {
    glm::vec3 targetDir = glm::normalize(move);

    // Current direction
    glm::vec3 currentDir = orientation * glm::vec3(0,0,1);

    // Necesary rotation (delta)
    glm::quat targetRot = glm::rotation(currentDir, targetDir);

    // angular velocity
    float maxAngle = glm::radians(720.0f) * deltaTime;

    // Turn rotation into angle
    float angle = glm::angle(targetRot);

    if (angle > 0.001f)
    {
      float t = glm::min(1.0f, maxAngle / angle);

      // Interpolate rotation (slerp)
      glm::quat step = glm::slerp(glm::quat(1,0,0,0), targetRot, t);

      // Apply incremental rotation
      orientation = glm::normalize(step * orientation);
    }
  }

  // Position increment
  // X Axis
  glm::vec3 newPosX = position;
  newPosX.x += move.x * velocity;

  if (!CheckCollision(newPosX, map))
    position.x = newPosX.x;

  // Z Axis
  glm::vec3 newPosZ = position;
  newPosZ.z += move.z * velocity;

  if (!CheckCollision(newPosZ, map))
    position.z = newPosZ.z;
}

void Player::ProcessKeyboardFPS(Player_Movement direction, glm::vec3 cameraFront, glm::vec3 cameraRight, GLfloat deltaTime, const Map& map)
{
  GLfloat velocity = this->speed * deltaTime;
  glm::vec3 move(0.0f);

  // Anular Y y normalizar
  glm::vec3 front = glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));
  glm::vec3 right = glm::normalize(glm::vec3(cameraRight.x, 0.0f, cameraRight.z));

  if (direction == NORTH)
    move += front;

  if (direction == SOUTH)
    move -= front;

  if (direction == EAST)
    move += right;

  if (direction == WEST)
    move -= right;

  if (glm::length(move) > 0.0f)
  {
    move = glm::normalize(move);
    
    // Progressive rotation
    glm::vec3 targetDir = move;
    glm::vec3 currentDir = orientation * glm::vec3(0,0,1);
    glm::quat targetRot = glm::rotation(currentDir, targetDir);
    float maxAngle = glm::radians(720.0f) * deltaTime;
    float angle = glm::angle(targetRot);

    if (angle > 0.001f)
    {
      float t = glm::min(1.0f, maxAngle / angle);
      glm::quat step = glm::slerp(glm::quat(1,0,0,0), targetRot, t);
      orientation = glm::normalize(step * orientation);
    }
  }

  // Position increment
  // X Axis
  glm::vec3 newPosX = position;
  newPosX.x += move.x * velocity;

  if (!CheckCollision(newPosX, map))
    position.x = newPosX.x;

  // Z Axis
  glm::vec3 newPosZ = position;
  newPosZ.z += move.z * velocity;

  if (!CheckCollision(newPosZ, map))
    position.z = newPosZ.z;
}

void Player::setCanPassBomb(bool value)
{
  canPassBomb = value;
}

bool Player::CheckCollision(glm::vec3 newPos, const Map& map)
{
  // The "radius" of the player (hitbox)
  float half = halfHitbox;

  // Flag to know if the player is inside the bomb
  bool touchingBomb = false;

  // 4 corners of hitbox (real position)
  glm::vec3 corners[4] = {
    {newPos.x - half, 0.0f, newPos.z - half},
    {newPos.x + half, 0.0f, newPos.z - half},
    {newPos.x - half, 0.0f, newPos.z + half},
    {newPos.x + half, 0.0f, newPos.z + half}
  };

  // Check collision for each corner
  for (int i = 0; i < 4; i++)
  {
    // Convert real position into logic for map matrix
    MapIndices indices = map.toMapIndices(corners[i]);
    int col = indices.col;
    int row = indices.row;

    int cell = map.getCell(row, col);

    if (cell == 5)
    {
      touchingBomb = true;

      if(!canPassBomb)
        return true;
    }
    else if (cell != 0 && cell != 6 && cell != 7 && cell != 8) // Collision omitting flame, door, and powerup
      return true;
  }

  if(!touchingBomb)
    canPassBomb = false;

  return false;
}

void Player::resetTo(glm::vec3 pos, glm::vec3 dir) {
  dir = glm::normalize(dir);
  position = pos;
  orientation = glm::rotation(glm::vec3(0,0,1), dir);
}
