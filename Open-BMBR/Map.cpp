#include "Map.hpp"
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <utility>

// Probability of a block to be destructible
const int destructibleProb = 60;

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

        int numPowerUps = (rand() % 2) + 1;

        for (int i = 0; i < numPowerUps && !destructibleBricks.empty(); i++) {

            int powerIndex = rand() % destructibleBricks.size();

            int r = destructibleBricks[powerIndex].first;
            int c = destructibleBricks[powerIndex].second;

            mapGrid[r][c] = 4; // powerup

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
            else if (mapGrid[row][col] == 4)
                std::cout << "PP";
            else if (mapGrid[row][col] == 5)
                std::cout << "BB";
            else if (mapGrid[row][col] == 6)
                std::cout << "FF";
            else if (mapGrid[row][col] == 7)
                std::cout << "DD"; // Door revealed
            else if (mapGrid[row][col] == 8)
                std::cout << "pu"; // Powerup revealed
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

void Map::setFire(MapIndices indices)
{
  int row = indices.row;
  int col = indices.col;

  mapGrid[row][col] = 6; // Set the center

  MapIndices neighbors[4] = {
    {row - 1, col},
    {row + 1, col},
    {row, col - 1},
    {row, col + 1}
  };

  // For each neighbor
  for (int i = 0; i < 4; i++)
  {
    int cell = getCell(neighbors[i].row, neighbors[i].col); // neighbor cell
    if (cell == 0) // Only spread fire to empty air cells
      mapGrid[neighbors[i].row][neighbors[i].col] = 6;
    // Cells 7 (door) and 8 (powerup) are NOT overwritten by fire
  }
}

void Map::detonateBomb(MapIndices indices)
{
  int row = indices.row;
  int col = indices.col;
  // Bomb neighbors cells
  MapIndices neighbors[4] = {
    {row - 1, col},
    {row + 1, col},
    {row, col - 1},
    {row, col + 1}
  };

  // For each neighbor
  for (int i = 0; i < 4; i++)
  {
    int cell = getCell(neighbors[i].row, neighbors[i].col); // neighbor cell
    if (cell == 2) // A regular destructible block — clear it
      mapGrid[neighbors[i].row][neighbors[i].col] = 0;
    else if (cell == 3) // Hidden exit (door) — reveal it
      mapGrid[neighbors[i].row][neighbors[i].col] = 7;
    else if (cell == 4) // Hidden powerup — reveal it
      mapGrid[neighbors[i].row][neighbors[i].col] = 8;
  }

  mapGrid[row][col] = 0; // Destroy bomb
}

void Map::extinguishFire(MapIndices indices)
{
  int row = indices.row;
  int col = indices.col;

  mapGrid[row][col] = 0;

  MapIndices neighbors[4] = {
    {row - 1, col},
    {row + 1, col},
    {row, col - 1},
    {row, col + 1}
  };

  // For each neighbor
  for (int i = 0; i < 4; i++)
  {
    int cell = getCell(neighbors[i].row, neighbors[i].col); // neighbor cell
    if (cell == 6) // Fire cell
      mapGrid[neighbors[i].row][neighbors[i].col] = 0; // Set air id
  }
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
