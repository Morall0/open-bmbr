#include "Player.hpp"
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
  // X Axe
  glm::vec3 newPosX = position;
  newPosX.x += move.x * velocity;

  if (!CheckCollision(newPosX, map))
    position.x = newPosX.x;

  // Z Axe
  glm::vec3 newPosZ = position;
  newPosZ.z += move.z * velocity;

  if (!CheckCollision(newPosZ, map))
    position.z = newPosZ.z;
}

bool Player::CheckCollision(glm::vec3 newPos, const Map& map)
{
  // The "radius" of the player (hitbox)
  float half = halfHitbox;

  float half_cols = (map.getTotalCols() + 1) / 2.0f;
  float half_rows = (map.getTotalRows() + 1) / 2.0f;

  // 4 corners of hitbox (real position)
  glm::vec3 corners[4] = {
    {newPos.x - half, 0.0f, newPos.z - half},
    {newPos.x + half, 0.0f, newPos.z - half},
    {newPos.x - half, 0.0f, newPos.z + half},
    {newPos.x + half, 0.0f, newPos.z + half}
  };

  for (int i = 0; i < 4; i++)
  {
    // Convert real position into logic for map matrix
    int col = (int)std::round(corners[i].x + (half_cols - 1));
    int row = (int)std::round(corners[i].z + (half_rows - 1));

    int cell = map.getCell(row, col);

    if (cell != 0) // Collision
      return true;
  }

  return false;
}
