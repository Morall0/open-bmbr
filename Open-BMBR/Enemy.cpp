#include "Enemy.hpp"
#include "Map.hpp"
#include <cstdlib>
#include <cmath>
#include <vector>
#include <map>
#include <set>

//Constructor
Enemy::Enemy(glm::vec3 startPos, EnemyType type) 
    : position(startPos), targetPosition(startPos), type(type), isMoving(false) 
{
    orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    
    // Set speed based on type
    if (type == EnemyType::BALLOM) {
        speed = 4.0f * 0.3f;
    } else {
        speed = 4.0f * 0.5f; // ONIL
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
    int numOnils = rand() % 3; // 0 to 2 Onils
    
    int rows = map.getTotalRows();
    int cols = map.getTotalCols();
    float half_cols = (cols + 1) / 2.0f;
    float half_rows = (rows + 1) / 2.0f;

    // Use DFS to find the entire safe zone connected to player's spawn (0,0)
    std::vector<std::vector<bool>> safeZone(rows, std::vector<bool>(cols, false));
    std::vector<std::pair<int, int>> stack;
    stack.push_back({0, 0});
    
    while (!stack.empty()) {
        auto [r, c] = stack.back();
        stack.pop_back();
        
        if (r < 0 || r >= rows || c < 0 || c >= cols) continue;
        if (map.getCell(r, c) != 0) continue;
        if (safeZone[r][c]) continue;
        
        safeZone[r][c] = true;
        
        stack.push_back({r - 1, c});
        stack.push_back({r + 1, c});
        stack.push_back({r, c - 1});
        stack.push_back({r, c + 1});
    }

    std::vector<std::pair<int, int>> validSpawnSpots;
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            // Ignore player spawn safe zone (all cells connected to 0,0)
            if (safeZone[r][c]) continue;

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
        
        EnemyType type = (i < numOnils) ? EnemyType::ONIL : EnemyType::BALLOM;
        enemies.push_back(Enemy(glm::vec3(xPos, yPos, zPos), type));
        
        // Remove to avoid spawning another enemy in the exact same spot
        validSpawnSpots.erase(validSpawnSpots.begin() + idx);
    }

    return enemies;
}

void Enemy::Update(float deltaTime, const Map& map, glm::vec3 playerPos) {
    if (!isMoving) {
        pickNewTarget(map, playerPos);
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

void Enemy::pickNewTarget(const Map& map, glm::vec3 playerPos) {
    float half_cols = (map.getTotalCols() + 1) / 2.0f;
    float half_rows = (map.getTotalRows() + 1) / 2.0f;

    int col = (int)std::round(position.x + (half_cols - 1));
    int row = (int)std::round(position.z + (half_rows - 1));

    int playerCol = (int)std::round(playerPos.x + (half_cols - 1));
    int playerRow = (int)std::round(playerPos.z + (half_rows - 1));

    std::vector<std::pair<int, int>> validNeighbors;
    if (map.getCell(row - 1, col) == 0) validNeighbors.push_back({row - 1, col});
    if (map.getCell(row + 1, col) == 0) validNeighbors.push_back({row + 1, col});
    if (map.getCell(row, col - 1) == 0) validNeighbors.push_back({row, col - 1});
    if (map.getCell(row, col + 1) == 0) validNeighbors.push_back({row, col + 1});

    if (validNeighbors.empty()) return;

    int targetRow = -1;
    int targetCol = -1;

    // Onil specific AI
    if (type == EnemyType::ONIL) {
        int chance = rand() % 100;
        if (chance < 60) { // 60% chance to chase
            auto nextMove = getNextAStarMove(map, row, col, playerRow, playerCol);
            if (nextMove.first != -1) {
                targetRow = nextMove.first;
                targetCol = nextMove.second;
            }
        }
    }

    // Default or Fallback random choice
    if (targetRow == -1) {
        int rIndex = rand() % validNeighbors.size();
        targetRow = validNeighbors[rIndex].first;
        targetCol = validNeighbors[rIndex].second;
    }

    // Convert target logical cell back to physical position
    targetPosition.x = targetCol - (half_cols - 1);
    targetPosition.z = targetRow - (half_rows - 1);
    targetPosition.y = position.y;

    isMoving = true;
}

std::pair<int, int> Enemy::getNextAStarMove(const Map& map, int startRow, int startCol, int targetRow, int targetCol) {
    if (targetRow < 0 || targetRow >= map.getTotalRows() || targetCol < 0 || targetCol >= map.getTotalCols()) {
        return {-1, -1};
    }
    
    if (startRow == targetRow && startCol == targetCol) {
        return {-1, -1};
    }

    using Point = std::pair<int, int>;
    
    auto heuristic = [](Point a, Point b) {
        return std::abs(a.first - b.first) + std::abs(a.second - b.second);
    };

    std::map<Point, Point> cameFrom;
    std::map<Point, int> gScore;
    
    auto comp = [&](Point a, Point b) {
        int fA = gScore[a] + heuristic(a, {targetRow, targetCol});
        int fB = gScore[b] + heuristic(b, {targetRow, targetCol});
        if (fA == fB) return a < b; // Tie breaker
        return fA < fB;
    };
    std::set<Point, decltype(comp)> openSet(comp);

    Point start = {startRow, startCol};
    Point goal = {targetRow, targetCol};

    gScore[start] = 0;
    openSet.insert(start);

    while (!openSet.empty()) {
        Point current = *openSet.begin();
        openSet.erase(openSet.begin());

        if (current == goal) {
            // Reconstruct path
            Point curr = goal;
            while (cameFrom.find(curr) != cameFrom.end()) {
                if (cameFrom[curr] == start) {
                    return curr; // First step from start
                }
                curr = cameFrom[curr];
            }
            return {-1, -1};
        }

        std::vector<Point> neighbors = {
            {current.first - 1, current.second},
            {current.first + 1, current.second},
            {current.first, current.second - 1},
            {current.first, current.second + 1}
        };

        for (const auto& neighbor : neighbors) {
            if (map.getCell(neighbor.first, neighbor.second) != 0) continue;

            int tentative_gScore = gScore[current] + 1;

            if (gScore.find(neighbor) == gScore.end() || tentative_gScore < gScore[neighbor]) {
                cameFrom[neighbor] = current;
                gScore[neighbor] = tentative_gScore;
                openSet.insert(neighbor);
            }
        }
    }

    return {-1, -1};
}
