#ifndef ENEMY_HPP
#define ENEMY_HPP

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>
#include "Map.hpp"

enum class EnemyType {
    BALLOM,
    ONIL
};

class Enemy {
public:
    Enemy(glm::vec3 startPos, EnemyType type);

    void Update(float deltaTime, const Map& map, glm::vec3 playerPos);
    glm::vec3 getPosition() const;
    glm::quat getOrientation() const;
    EnemyType getType() const;
    bool getIsDead() const;
    float getDeathScale() const;
    bool getIsMoving() const;

    static std::vector<Enemy> SpawnEnemies(const Map& map, int minEnemies, int maxEnemies);

protected:
    glm::vec3 position;
    glm::quat orientation;
    glm::vec3 targetPosition;
    EnemyType type;
    float speed;
    bool isMoving;
    bool isDead;
    float deathTimer;   // Tiempo acumulado desde que murió
    float deathScale;   // Escala visual (1.0 -> 0.0 durante la animación de muerte)

    void pickNewTarget(const Map& map, glm::vec3 playerPos);
    std::pair<int, int> getNextAStarMove(const Map& map, int startRow, int startCol, int targetRow, int targetCol);
};

#endif
