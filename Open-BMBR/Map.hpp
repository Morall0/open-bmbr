#ifndef MAP_HPP
#define MAP_HPP

#include <vector>
#include <utility>

class Map {
private:
    int rows;
    int cols;
    int **mapGrid;

    std::vector<std::pair<int,int>> destructibleBricks;

public:
    Map(int rows, int cols);
    ~Map();

    void genMap();
    void genHidden();
    void printMap();
    int getCell(int row, int col);
};

#endif
