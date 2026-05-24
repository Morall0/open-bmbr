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
  float  ambientStrength = 0.2f;
  float  specularStrength = 0.1f;
  float  shininess = 16.0f;
  float  uvScaleX = 1.0f;
  float  uvScaleY = 1.0f;
};

// Structure for Map Drawing functions
struct MapMaterials {
  Material ground_mat;
  Material wall_mat;
  Material hardblock_mat;
  Material brick_mat;
  Material bomb_pu_mat;
  Material fire_pu_mat;
  Material speed_pu_mat;
};

class Drawer {
  public:
    Drawer(Shader& lightingShader, Shader& skeletalAnimShader, Shader& modelLoadingShader, Shader& uiShader);

    // Initialization
    void InitMapMaterials();
    void InitUI();

    // Static objects drawing
    void DrawMap(const Map& map, GLfloat currentFrame);
    void DrawDoor(const Map& map, Model& doorBaseModel, Model& doorModel, Animator& animator, bool locked);

    // Dynamic objects drawing
    void DrawBomb(std::vector<Bomb>& bombs, Map& map, GLfloat currentTime, Model& bombModel);
    void DrawFire(std::vector<Bomb>& bombs, Map& map, GLfloat currentTime, Model& fireModel, Animator& fireAnimator);

    // Characters drawing
    void DrawRobot(Player& bomberman, Model& robot, Animator& animator, bool isFirstPerson);
    void DrawEnemies(const std::vector<Enemy>& enemies, GLfloat currentFrame, Model& ballomModel, Model& onilBodyModel, Model& onilLeftFtModel, Model& onilRightFtModel);

    // Light Updates
    void SetupLights(std::vector<Bomb>& bombs, GLfloat currentFrame);

    // UI Drawing
    void DrawPauseMenu(bool isPaused, int selection, float deltaTime, int screenWidth, int screenHeight);

  private:
    Shader& lightingShader;
    Shader& skeletalAnimShader;
    Shader& modelLoadingShader;
    Shader& uiShader;

    MapMaterials map_materials;

    GLuint doorAlternativeTex;

    // UI state
    GLuint uiVAO, uiVBO;
    GLuint menuTexture;
    GLuint logoTexture;
    float currentSelectorY = 0.0f;
    float menuScale = 0.0f;

    // Helper functions
    void DrawFloor();
    void DrawWalls();
    void DrawMapBlocks(const Map& map, GLfloat currentFrame);

    void ApplyTexture(const Material& material, Shader& shader);
    GLuint LoadTexture2D(const char* path);
    GLuint CreateSolidTexture(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
};

#endif
