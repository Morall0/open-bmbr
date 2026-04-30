#include "Map.hpp"
#include <iostream>
#include <cstdlib>

// Probability of a block to be destructible
const int destructibleProb = 60;

// Constructor
Map::Map(int rows, int cols)
    : rows(rows), cols(cols),
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
void Map::printMap() {
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
