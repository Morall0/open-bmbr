#include "Map.hpp"
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <utility>

// Probability of a block to be destructible
const int destructibleProb = 50;

// Constructor
Map::Map(int rows, int cols)
    : rows(rows), cols(cols),
      half_cols((cols + 1) / 2.0f),
      half_rows((rows + 1) / 2.0f),
      mapGrid(rows, std::vector<int>(cols, 0))
{
}

// Generate the map matrix
void Map::genMap() {
    destructibleBricks.clear();
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {

            // Secure zone
            if ((row == 0 && col == 0) ||
                (row == 0 && col == 1) ||
                (row == 1 && col == 0)) {
                mapGrid[row][col] = 0;
                continue;
            }

            if (row % 2 == 1 && col % 2 == 1) {
                mapGrid[row][col] = 1; // Fixed cols
            } else {
                if (rand() % 100 < destructibleProb) {
                    mapGrid[row][col] = 2; // Destructible block
                    destructibleBricks.push_back(std::make_pair(row, col));
                } else {
                    mapGrid[row][col] = 0;
                }
            }
        }
    }
}

// Generate the blocks inside the map
void Map::genHidden() {

    if (!destructibleBricks.empty()) {
        int exitIndex = rand() % destructibleBricks.size();

        int r = destructibleBricks[exitIndex].first;
        int c = destructibleBricks[exitIndex].second;

        mapGrid[r][c] = 3; // exit

        destructibleBricks.erase(destructibleBricks.begin() + exitIndex);
    }

    if (!destructibleBricks.empty()) {

        int numPowerUps = (rand() % 3) + 2; // 2 to 4 powerups

        for (int i = 0; i < numPowerUps && !destructibleBricks.empty(); i++) {

            int powerIndex = rand() % destructibleBricks.size();

            int r = destructibleBricks[powerIndex].first;
            int c = destructibleBricks[powerIndex].second;

            int powerType = rand() % 3;
            if (powerType == 0)
                mapGrid[r][c] = 4; // Hidden Bomb PU
            else if (powerType == 1)
                mapGrid[r][c] = 9; // Hidden Fire PU
            else
                mapGrid[r][c] = 10; // Hidden Speed PU

            destructibleBricks.erase(destructibleBricks.begin() + powerIndex);
        }
    }
}

// Prints map to console for debugging
void Map::printMap() const {
    std::cout << "\n--- MAP GENERATED ---\n";

    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {

            if (mapGrid[row][col] == 0)
                std::cout << "  ";
            else if (mapGrid[row][col] == 1)
                std::cout << "[]";
            else if (mapGrid[row][col] == 2)
                std::cout << "##";
            else if (mapGrid[row][col] == 3)
                std::cout << "EE";
            else if (mapGrid[row][col] == 4 || mapGrid[row][col] == 9 || mapGrid[row][col] == 10)
                std::cout << "PP";
            else if (mapGrid[row][col] == 5)
                std::cout << "BB";
            else if (mapGrid[row][col] == 6)
                std::cout << "FF";
            else if (mapGrid[row][col] == 7)
                std::cout << "DD"; // Door revealed
            else if (mapGrid[row][col] == 8 || mapGrid[row][col] == 11 || mapGrid[row][col] == 12)
                std::cout << "pu"; // Powerups revealed
        }

        std::cout << std::endl;
    }

    std::cout << "---------------------\n";
}

// Get the given map cell
int Map::getCell(int row, int col) const
{
    if(row < 0 || row >= rows || col < 0 || col >= cols)
        return 1; // Out of map bounds

    return mapGrid[row][col];
}

// Get the total number of rows
int Map::getTotalRows() const
{
  return rows;
}

// Get the total number cols
int Map::getTotalCols() const
{
  return cols;
}

void Map::setBomb(MapIndices indices)
{
  mapGrid[indices.row][indices.col] = 5;
}

void Map::detonateBomb(MapIndices indices, int fireRadius)
{
  int row = indices.row;
  int col = indices.col;

  mapGrid[row][col] = 6; // Set the center to fire

  int dRow[] = {-1, 1, 0, 0};
  int dCol[] = {0, 0, -1, 1};

  for (int i = 0; i < 4; i++) {
    for (int step = 1; step <= fireRadius; step++) {
      int r = row + dRow[i] * step;
      int c = col + dCol[i] * step;
      int cell = getCell(r, c);

      if (cell == 1) break; // Hard block stops explosion

      if (cell == 2) { mapGrid[r][c] = 6; break; } // Regular brick becomes fire, stops
      if (cell == 3) { mapGrid[r][c] = 7; break; } // Hidden door becomes revealed, stops
      if (cell == 4) { mapGrid[r][c] = 8; break; } // Hidden Bomb PU
      if (cell == 9) { mapGrid[r][c] = 11; break; } // Hidden Fire PU
      if (cell == 10) { mapGrid[r][c] = 12; break; } // Hidden Speed PU

      if (cell == 8 || cell == 11 || cell == 12) {
        mapGrid[r][c] = 6; // Destroy revealed powerups
      } else if (cell == 0) {
        mapGrid[r][c] = 6; // Empty space becomes fire
      }
    }
  }
}

void Map::extinguishFire(MapIndices indices, int fireRadius)
{
  int row = indices.row;
  int col = indices.col;

  mapGrid[row][col] = 0;

  int dRow[] = {-1, 1, 0, 0};
  int dCol[] = {0, 0, -1, 1};

  for (int i = 0; i < 4; i++) {
    for (int step = 1; step <= fireRadius; step++) {
      int r = row + dRow[i] * step;
      int c = col + dCol[i] * step;
      int cell = getCell(r, c);

      if (cell == 6) {
        mapGrid[r][c] = 0;
      } else if (cell != 0) {
        break; // Stop raycast at any non-empty non-fire block
      }
    }
  }
}

int Map::collectPowerup(MapIndices indices)
{
  int row = indices.row;
  int col = indices.col;

  if (row < 0 || row >= rows || col < 0 || col >= cols)
    return 0;

  int cell = mapGrid[row][col];
  if (cell == 8 || cell == 11 || cell == 12) {
    mapGrid[row][col] = 0; // Remove powerup from map
    return cell;
  }
  return 0;
}

MapIndices Map::toMapIndices(glm::vec3 position) const
{
  MapIndices indices;

  indices.col = (int)std::round(position.x + (half_cols - 1));
  indices.row = (int)std::round(position.z + (half_rows - 1));

  return indices;
}

// Returns true if the door has been revealed (cell type 7 exists in the grid)
bool Map::hasDoorRevealed() const
{
  for (int r = 0; r < rows; r++)
    for (int c = 0; c < cols; c++)
      if (mapGrid[r][c] == 7)
        return true;
  return false;
}

// Returns the map indices of the revealed door (cell 7).
// Check hasDoorRevealed() first before calling this.
MapIndices Map::getDoorPosition() const
{
  for (int r = 0; r < rows; r++)
    for (int c = 0; c < cols; c++)
      if (mapGrid[r][c] == 7)
        return {r, c};
  return {-1, -1};
}
