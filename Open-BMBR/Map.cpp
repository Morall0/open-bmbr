#include "Map.hpp"
#include <iostream>
#include <cstdlib>

// Constructor
Map::Map(int rows, int cols) {
    this->rows = rows;
    this->cols = cols;

    mapGrid = new int*[rows];

    for (int i = 0; i < rows; i++) {
        mapGrid[i] = new int[cols];

        for (int j = 0; j < cols; j++) {
            mapGrid[i][j] = 0;
        }
    }
}

// Destructor
Map::~Map() {
    for (int i = 0; i < rows; i++) {
        delete[] mapGrid[i];
    }

    delete[] mapGrid;
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
                if (rand() % 100 < 60) {
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

        mapGrid[r][c] = 3; // salida

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
int Map::getCell(int row, int col) {
  return mapGrid[row][col];
}
