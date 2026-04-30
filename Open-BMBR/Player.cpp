#include "Player.hpp"
#include <cmath>

const GLfloat SPEED = 6.0f;

// Constructor
Player::Player (glm::vec3 position, glm::vec2 direction) 
      : position(position), direction(glm::vec3(direction.x, 0.0f, direction.y)), speed(SPEED)
{
}

glm::vec3 Player::getPosition()
{
  return this->position;
}

glm::vec2 Player::getDirection()
{
  return glm::vec2(this->direction.x, this->direction.z);
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
