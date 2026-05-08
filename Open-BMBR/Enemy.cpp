#include "Enemy.hpp"
#include "Map.hpp"
#include <cstdlib>
#include <cmath>

//Constructor
Enemy::Enemy(glm::vec3 startPos, EnemyType type) 
    : position(startPos), targetPosition(startPos), type(type), isMoving(false) 
{
    orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    
    // Set speed based on type
    if (type == EnemyType::BALLOM) {
        speed = 4.0f * 0.4f;
    } else {
        speed = 4.0f * 0.6f; // ONIL
    }
}

glm::vec3 Enemy::getPosition() const {
    return position;
}

glm::quat Enemy::getOrientation() const {
    return orientation;
}

EnemyType Enemy::getType() const {
    return type;
}

std::vector<Enemy> Enemy::SpawnEnemies(const Map& map, int minEnemies, int maxEnemies) {
    std::vector<Enemy> enemies;
    int numEnemies = (rand() % (maxEnemies - minEnemies + 1)) + minEnemies;
    
    int rows = map.getTotalRows();
    int cols = map.getTotalCols();
    float half_cols = (cols + 1) / 2.0f;
    float half_rows = (rows + 1) / 2.0f;

    std::vector<std::pair<int, int>> validSpawnSpots;
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            // Ignore player spawn safe zone
            if ((r == 0 && c == 0) || (r == 0 && c == 1) || (r == 1 && c == 0)) continue;

            if (map.getCell(r, c) == 0) {
                // Check neighbors to see if it's an open area (has several empty cells)
                int emptyNeighbors = 0;
                if (map.getCell(r - 1, c) == 0) emptyNeighbors++;
                if (map.getCell(r + 1, c) == 0) emptyNeighbors++;
                if (map.getCell(r, c - 1) == 0) emptyNeighbors++;
                if (map.getCell(r, c + 1) == 0) emptyNeighbors++;

                // Require at least 2 empty neighbors
                if (emptyNeighbors >= 2) {
                    validSpawnSpots.push_back({r, c});
                }
            }
        }
    }

    // Pick random spots and spawn enemies
    for (int i = 0; i < numEnemies && !validSpawnSpots.empty(); i++) {
        int idx = rand() % validSpawnSpots.size();
        int r = validSpawnSpots[idx].first;
        int c = validSpawnSpots[idx].second;

        float xPos = c - (half_cols - 1);
        float zPos = r - (half_rows - 1);
        float yPos = -0.49f; // Same plane as player
        
        enemies.push_back(Enemy(glm::vec3(xPos, yPos, zPos), EnemyType::BALLOM));
        
        // Remove to avoid spawning another enemy in the exact same spot
        validSpawnSpots.erase(validSpawnSpots.begin() + idx);
    }

    return enemies;
}

void Enemy::Update(float deltaTime, const Map& map) {
    if (!isMoving) {
        pickNewTarget(map);
    }

    if (isMoving) {
        glm::vec3 moveDir = targetPosition - position;
        float dist = glm::length(moveDir);

        if (dist <= speed * deltaTime) {
            // We reached or will overshoot the target this frame
            position = targetPosition;
            isMoving = false;
        } else {
            // Move towards target
            moveDir = glm::normalize(moveDir);
            position += moveDir * (speed * deltaTime);

            // Update orientation to look at the movement direction
            glm::vec3 currentDir = orientation * glm::vec3(0.0f, 0.0f, 1.0f);
            glm::quat targetRot = glm::rotation(currentDir, moveDir);
            float angle = glm::angle(targetRot);
            if (angle > 0.001f) {
                float maxAngle = glm::radians(720.0f) * deltaTime;
                float t = glm::min(1.0f, maxAngle / angle);
                glm::quat step = glm::slerp(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), targetRot, t);
                orientation = glm::normalize(step * orientation);
            }
        }
    }
}

void Enemy::pickNewTarget(const Map& map) {
    // Current logical cell
    float half_cols = (map.getTotalCols() + 1) / 2.0f;
    float half_rows = (map.getTotalRows() + 1) / 2.0f;

    MapIndices indices = map.toMapIndices(position);
    int col = indices.col;
    int row = indices.row;

    // Possible valid neighbors (up, down, left, right)
    std::vector<std::pair<int, int>> validNeighbors;

    // Check North
    if (map.getCell(row - 1, col) == 0) validNeighbors.push_back({row - 1, col});
    // Check South
    if (map.getCell(row + 1, col) == 0) validNeighbors.push_back({row + 1, col});
    // Check West
    if (map.getCell(row, col - 1) == 0) validNeighbors.push_back({row, col - 1});
    // Check East
    if (map.getCell(row, col + 1) == 0) validNeighbors.push_back({row, col + 1});

    if (!validNeighbors.empty()) {
        int rIndex = rand() % validNeighbors.size();
        int targetRow = validNeighbors[rIndex].first;
        int targetCol = validNeighbors[rIndex].second;

        // Convert target logical cell back to physical position
        targetPosition.x = targetCol - (half_cols - 1);
        targetPosition.z = targetRow - (half_rows - 1);
        targetPosition.y = position.y;

        isMoving = true;
    }
}
