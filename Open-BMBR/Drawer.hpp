#ifndef DRAWER_HPP
#define DRAWER_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

#include "Shader.h"
#include "Map.hpp"
#include "Bomb.hpp"
#include "Enemy.hpp"
#include "Player.hpp"
#include "model_animation.h"
#include "animator.h"

// Constants for Map size
const int MAP_ROWS = 11;
const int MAP_COLS = 29;
const float HALF_ROWS = (MAP_ROWS + 1) / 2.0f;
const float HALF_COLS = (MAP_COLS + 1) / 2.0f;

// Structure for material
struct Material {
  GLuint diffuse;
  GLuint specular;
  GLuint ao;
  float shininess = 16.0f;
  float uvScaleX = 1.0f;
  float uvScaleY = 1.0f;
};

// Structure for Map Drawing functions
struct MapMaterials {
  Material ground_mat;
  Material wall_mat;
  Material brick_mat;
};

class Drawer {
  public:
    Drawer(Shader& lightingShader, Shader& skeletalAnimShader, Shader& modelLoadingShader);

    // Initialization
    void InitMapMaterials();

    // Static objects drawing
    void DrawMap(const Map& map);
    void DrawDoor(const Map& map, Model& hatchModel);

    // Dynamic objects drawing
    void DrawBomb(Bomb& bomb, Map& map, GLfloat currentTime, Model& bombModel);
    void DrawFire(Bomb& bomb, Map& map, GLfloat currentTime, Model& fireModel, Animator& fireAnimator);

    // Characters drawing
    void DrawRobot(Player& bomberman, Model& robot, Animator& animator, bool isFirstPerson);
    void DrawEnemies(const std::vector<Enemy>& enemies, GLfloat currentFrame, Model& ballomModel, Model& onilCuerpoModel, Model& onilPieIzqModel, Model& onilPieDerModel);

    // Light Updates
    void SetupLights(Bomb& bomb, GLfloat currentFrame);

  private:
    Shader& lightingShader;
    Shader& skeletalAnimShader;
    Shader& modelLoadingShader;

    MapMaterials map_materials;

    // Helper functions
    void DrawFloor();
    void DrawWalls();
    void DrawMapBlocks(const Map& map);

    void ApplyTexture(const Material& material, Shader& shader);
    GLuint LoadTexture2D(const char* path);
    GLuint CreateSolidTexture(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
};

#endif
