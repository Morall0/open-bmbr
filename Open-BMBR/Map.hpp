#ifndef MAP_HPP
#define MAP_HPP

#include <vector>
#include <utility>
#include <glm/glm.hpp>


struct MapIndices {
  int row, col;
} typedef MapIndices;

class Map {
private:
    int rows;
    int cols;
    float half_cols;
    float half_rows;
    std::vector<std::vector<int>> mapGrid;

    std::vector<std::pair<int,int>> destructibleBricks;

public:
    Map(int rows, int cols);

    void genMap();
    void genHidden();
    void printMap() const;
    int getCell(int row, int col) const;
    int getTotalRows() const;
    int getTotalCols() const;
    void setBomb(MapIndices indices);
    void setFire(MapIndices indices);
    void detonateBomb(MapIndices indices);
    void extinguishFire(MapIndices indices);
    MapIndices toMapIndices(glm::vec3 position) const;
    bool hasDoorRevealed() const;
    MapIndices getDoorPosition() const;
};

#endif
