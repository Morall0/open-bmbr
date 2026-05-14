#include <cmath>
#include <cstdlib>
#include <ctime>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <iostream>

// GLEW
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Other Libs
// #define STB_IMAGE_IMPLEMENTATION
#include "bone.h"
#include "stb_image.h"

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Load Models
// #include "SOIL2/SOIL2.h"

// Other includes
#include "Bomb.hpp"
#include "Camera.h"
#include "Map.hpp"
#include "animation.h"
#include "animator.h"
// #include "Model.h"
#include "Enemy.hpp"
#include "Player.hpp"
#include "Shader.h"
#include "model_animation.h"

// Structure for material
struct Material {
  GLuint diffuse;
  GLuint specular;

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

struct AnimationsSet {
  Animation idleAnim;
  Animation walkingAnim;
  Animation deadAnim;
  Animation punchAnim;
};

// Function prototypes
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow *window, double xPos, double yPos);
void DoMovement(Player& bomberman, Map& map, std::vector<Enemy>& enemies, Bomb& bomb, Animator& animator, AnimationsSet& animations);

// Texture functions
GLuint LoadTexture2D(const char *path);
GLuint CreateSolidTexture(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
void ApplyTexture(const Material& material, Shader& lightingShader);

// Map Draw Functions
void DrawMap(const Map& map, Shader& lightingShader, const MapMaterials& map_materials);
void DrawFloor(Shader& lightingShader, const Material& ground_mat);
void DrawWalls(Shader& lightingShader, const Material& wall_mat);
void DrawMapBlocks(const Map& map, Shader& lightingShader, const MapMaterials& map_materials);

// Bomb Draw Function
void DrawBomb(Bomb& bomb, Map& map, GLfloat currentTime, Shader& lightingShader, Model& bombModel, Shader& modelLoadingShader);
void DrawFire(Bomb& bomb, Map& map, GLfloat currentTime, Model& fireModel, Shader& skeletalAnimShader);

void SelectAnimation(Animator& animator, AnimationsSet& animations);

// Function to reset the game
void restart(Player& bomberman, Map& map, std::vector<Enemy>& enemies);

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true;

enum CameraMode { MODE_FREE, MODE_FIRST_PERSON, MODE_SIDE_SCROLL };
CameraMode currentCameraMode = MODE_SIDE_SCROLL;

// Map dimensions
const int ROWS = 11, COLS = 29;

const float half_rows = (ROWS + 1) / 2.0f;
const float half_cols = (COLS + 1) / 2.0f;

bool moving = false;
bool playerIsAlive = true;

float vertices[] = {
  // Vertex coords     // Normal cords      // Texcoords 
  -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // Back
   0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
   0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
   0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
  -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
  -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f,

  -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // Front
   0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f,
   0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
   0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
  -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f,
  -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,

  -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // Left
  -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
  -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
  -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
  -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
  -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,

   0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // Right
   0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
   0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
   0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
   0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
   0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,

  -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,// Bottom
   0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
   0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f,
   0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f,
  -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
  -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,

  -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // Up
   0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
   0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
   0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
  -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f,
  -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f
};

// Deltatime
GLfloat deltaTime = 0.0f; // Time between current frame and last frame
GLfloat lastFrame = 0.0f; // Time of last frame

int main() {
  // Init GLFW
  glfwInit();
  // Set all the required options for GLFW
  // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  // glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  // Create a GLFWwindow object that we can use for GLFW's functions
  GLFWwindow *window =
      glfwCreateWindow(WIDTH, HEIGHT, "Open-BMBR", nullptr, nullptr);

  if (nullptr == window) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();

    return EXIT_FAILURE;
  }

  glfwMakeContextCurrent(window);

  glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

  // Set the required callback functions
  glfwSetKeyCallback(window, KeyCallback);
  glfwSetCursorPosCallback(window, MouseCallback);

  // GLFW Options
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // Set this to true so GLEW knows to use a modern approach to retrieving
  // function pointers and extensions
  glewExperimental = GL_TRUE;

  // Initialize GLEW to setup the OpenGL Function pointers
  if (GLEW_OK != glewInit()) {
    std::cout << "Failed to initialize GLEW" << std::endl;
    return EXIT_FAILURE;
  }

  // OpenGL options
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);

  // Define the viewport dimensions
  glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

  // SHADERS
  Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");
  Shader skeletalAnimShader("Shader/skeletal.vs", "Shader/skeletal.frag");
  Shader modelLoadingShader("Shader/modelLoading.vs", "Shader/modelLoading.frag");

  stbi_set_flip_vertically_on_load(true);

  // MODEL LOADING
  Model robot("Models/Robot.gltf");

  AnimationsSet animations = {
    Animation("Models/Robot.gltf", &robot, 2),
    Animation("Models/Robot.gltf", &robot, 10),
    Animation("Models/Robot.gltf", &robot, 1),
    Animation("Models/Robot.gltf", &robot, 5)
  };

  Animator animator(&animations.idleAnim, true); // Setting base animation

  Model fire("Models/Fire.gltf");
  Animation fire_anim("Models/Fire.gltf", &fire, -1);
  Animator fire_animator(&fire_anim, true);

  Model ballomModel("Models/Ballom.obj");
  Model onilCuerpoModel("Models/onil_cuerpo.obj");
  Model onilPieIzqModel("Models/onil_pie_izquierdo.obj");
  Model onilPieDerModel("Models/onil_pie_derecho.obj");
  Model bombModel("Models/bomb.obj");

  // First, set the container's VAO (and VBO)
  GLuint VBO, VAO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  // Position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)0);
  glEnableVertexAttribArray(0);
  // normal attribute
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  // Texture Coordinate attribute
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)(6 * sizeof(GLfloat)));
  glEnableVertexAttribArray(2);
  glBindVertexArray(0);

  // Materials and textures
  // Texture loading
  GLuint ground_texture = LoadTexture2D("images/aerial_rocks_02_diff_1k.png");
  GLuint ground_specular = LoadTexture2D("images/aerial_rocks_02_rough_1k.png");
  GLuint wall_texture = LoadTexture2D("images/native_wall_1.png");
  // Texture creation
  GLuint brick_texture = CreateSolidTexture(64, 64, 64, 255); // Dark gray for destructible bricks

  // Material
  Material ground_mat = {ground_texture, ground_specular, 16.0f, 10.33f, 4.33f};
  Material wall_mat = {wall_texture, 0, 16.0f};
  Material brick_mat = {brick_texture, 0, 16.0f};

  // Configuring lightingShader
  lightingShader.use();

  // Material properties
  lightingShader.setInt("material.diffuse", 0);
  lightingShader.setInt("material.specular", 1);

  // Dir light properties
  lightingShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
  lightingShader.setVec3("dirLight.ambient",   0.3f, 0.3f, 0.3f);
  lightingShader.setVec3("dirLight.diffuse",   0.5f, 0.5f, 0.5f);
  lightingShader.setVec3("dirLight.specular",  0.0f, 0.0f, 0.0f);

  // Apply the same directional light to the skeletal and model-loading shaders
  skeletalAnimShader.use();
  skeletalAnimShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
  skeletalAnimShader.setVec3("dirLight.ambient",   0.3f, 0.3f, 0.3f);
  skeletalAnimShader.setVec3("dirLight.diffuse",   0.5f, 0.5f, 0.5f);
  skeletalAnimShader.setVec3("dirLight.specular",  0.8f, 0.8f, 0.8f);

  modelLoadingShader.use();
  modelLoadingShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
  modelLoadingShader.setVec3("dirLight.ambient",   0.3f, 0.3f, 0.3f);
  modelLoadingShader.setVec3("dirLight.diffuse",   0.5f, 0.5f, 0.5f);
  modelLoadingShader.setVec3("dirLight.specular",  0.8f, 0.8f, 0.8f);

  lightingShader.use(); // restore active shader

  // Initialize random seed
  srand(static_cast<unsigned int>(time(NULL)));

  // Map
  // Create map logic grid
  Map map(ROWS, COLS);
  map.genMap();
  map.genHidden();
  // map.printMap(); // Used to print map to console for debugging

  // Spawning enemies
  std::vector<Enemy> enemies = Enemy::SpawnEnemies(map, 6, 8);

  // Set map assets for drawing
  MapMaterials map_materials;
  map_materials.ground_mat = ground_mat;
  map_materials.wall_mat = wall_mat;
  map_materials.brick_mat = brick_mat;

  // Player
  // The initial position is in the secure zone map(0,0)
  Player bomberman(glm::vec3(-half_cols + 1, -0.49f, -half_rows + 1), glm::vec2(0.0f, 1.0f));

  // Initializing Bomb object with duration and y_pos
  Bomb bomb(2.5f, -0.2f);

  // Set the projection type and parameters
  glm::mat4 projection = glm::perspective(camera.GetZoom(), 
                                          (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 
                                          0.1f,
                                          100.0f);

  // Game loop
  CameraMode prevCameraMode = currentCameraMode;
  while (!glfwWindowShouldClose(window)) {
    // Calculate deltatime of current frame
    GLfloat currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // Check if any events have been activated (key pressed, mouse moved etc.)
    // and call corresponding response functions
    glfwPollEvents();
    DoMovement(bomberman, map, enemies, bomb, animator, animations);

    glm::vec3 player_position = bomberman.getPosition();

    for (auto it = enemies.begin(); it != enemies.end(); ) {
      it->Update(deltaTime, map, player_position);
      // Eliminar del vector solo cuando la animacion de muerte termino (escala = 0)
      if (it->getIsDead() && it->getDeathScale() <= 0.0f) {
        it = enemies.erase(it);
      } else {
        ++it;
      }
    }

    // Update Animation
    animator.UpdateAnimation(deltaTime);
    fire_animator.UpdateAnimation(deltaTime);

    // Clear the colorbuffer
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    lightingShader.use();

    // Detectar transicion de modo de camara
    if (currentCameraMode != prevCameraMode) {
      if (currentCameraMode == MODE_FREE) {
        // Centrada horizontal y verticalmente mirando hacia abajo
        camera.setPosition(glm::vec3(0.0f, 18.0f, 0.0f));
        camera.setYawPitch(-90.0f, -89.0f);
      } else if (currentCameraMode == MODE_FIRST_PERSON) {
        // Preservar el yaw actual (la direccion en que mira), forzar pitch=0 (horizonte)
        camera.setYawPitch(camera.getYaw(), 0.0f);
      }
      prevCameraMode = currentCameraMode;
    }

    if (currentCameraMode == MODE_FIRST_PERSON) {
      camera.setPosition(player_position + glm::vec3(0.0f, 0.8f, 0.0f));
    } else if (currentCameraMode == MODE_SIDE_SCROLL) {
      camera.UpdateSideScrollPosition(player_position);
    }

    // CAMERA ------
    glm::mat4 view = camera.GetViewMatrix();
    lightingShader.setMat4("view", view);
    lightingShader.setMat4("projection", projection); // Set projection
    lightingShader.setVec3("viewPos", camera.GetPosition());
    glBindVertexArray(VAO);

    // BOMB POINT LIGHT ------
    bool bombActive = bomb.getBombState();
    float bombBlinkScale = sin(currentFrame * 6.0f) * 0.05f;
    // La intensidad de la luz oscila igual que la escala de la bomba (0.0 a 1.0)
    float bombLightIntensity = 0.1f + bombBlinkScale * 10.0f; // base 0.5, pico ~1.0
    bombLightIntensity = glm::clamp(bombLightIntensity, 0.0f, 1.2f);

    lightingShader.setBool("bombLightActive", bombActive);
    if (bombActive) {
      glm::vec3 bpos = bomb.getBombPosition() + glm::vec3(0.0f, 0.5f, 0.0f);
      lightingShader.setVec3("bombLight.position", bpos);
      lightingShader.setFloat("bombLight.constant",  1.0f);
      lightingShader.setFloat("bombLight.linear",    0.35f);
      lightingShader.setFloat("bombLight.quadratic", 0.44f);
      lightingShader.setVec3("bombLight.ambient",  bombLightIntensity * 0.3f, bombLightIntensity * 0.15f, 0.0f);
      lightingShader.setVec3("bombLight.diffuse",  bombLightIntensity * 1.0f, bombLightIntensity * 0.5f,  0.0f);
      lightingShader.setVec3("bombLight.specular", bombLightIntensity * 0.5f, bombLightIntensity * 0.25f, 0.0f);
    }

    // Fire point light for walls/floor
    bool fireActive = bomb.isFireActive();
    lightingShader.setBool("fireLightActive", fireActive);
    if (fireActive) {
      glm::vec3 fpos = bomb.getBombPosition() + glm::vec3(0.0f, 0.3f, 0.0f);
      lightingShader.setVec3("fireLight.position",  fpos);
      lightingShader.setFloat("fireLight.constant",  1.0f);
      lightingShader.setFloat("fireLight.linear",    0.09f);
      lightingShader.setFloat("fireLight.quadratic", 0.032f);
      lightingShader.setVec3("fireLight.ambient",   0.8f,  0.3f,  0.0f);
      lightingShader.setVec3("fireLight.diffuse",   2.5f,  1.0f,  0.0f);
      lightingShader.setVec3("fireLight.specular",  1.0f,  0.5f,  0.0f);
    }

    // MAP ------
    DrawMap(map, lightingShader, map_materials);

    // BOMBS ------
    DrawBomb(bomb, map, currentFrame, lightingShader, bombModel, modelLoadingShader);

    // Check if the player dies from fire
    if (bomb.isFireActive() && bomb.checkCollision(player_position, map) && playerIsAlive) {
      playerIsAlive = false;
      animator.PlayAnimation(&animations.deadAnim, false);
    }

    // Check if the player dies from an enemy
    if (playerIsAlive) {
      for (const auto& enemy : enemies) {
        if (!enemy.getIsDead()) {
          float dist = glm::length(enemy.getPosition() - player_position);
          if (dist < 0.65f) {
            playerIsAlive = false;
            animator.PlayAnimation(&animations.deadAnim, false);
            break;
          }
        }
      }
    }


    // ROBOT ------
    skeletalAnimShader.use();

    // Send uniforms
    skeletalAnimShader.setMat4("view", view);
    skeletalAnimShader.setMat4("projection", projection);
    skeletalAnimShader.setVec3("viewPos", camera.GetPosition());

    // Fire point light (illuminates the robot when fire is active)
    skeletalAnimShader.setBool("fireLightActive", fireActive);
    if (fireActive) {
      glm::vec3 fpos = bomb.getBombPosition() + glm::vec3(0.0f, 0.3f, 0.0f);
      skeletalAnimShader.setVec3("fireLight.position",  fpos);
      skeletalAnimShader.setFloat("fireLight.constant",  1.0f);
      skeletalAnimShader.setFloat("fireLight.linear",    0.14f);
      skeletalAnimShader.setFloat("fireLight.quadratic", 0.07f);
      skeletalAnimShader.setVec3("fireLight.ambient",   0.6f,  0.25f, 0.0f);
      skeletalAnimShader.setVec3("fireLight.diffuse",   2.0f,  0.8f,  0.0f);
      skeletalAnimShader.setVec3("fireLight.specular",  1.0f,  0.5f,  0.0f);
    }

    // Bomb point light (illuminates the robot and while the bomb is ticking)
    skeletalAnimShader.setBool("bombLightActive", bombActive);
    if (bombActive) {
      glm::vec3 bpos = bomb.getBombPosition() + glm::vec3(0.0f, 0.5f, 0.0f);
      skeletalAnimShader.setVec3("bombLight.position",  bpos);
      skeletalAnimShader.setFloat("bombLight.constant",  1.0f);
      skeletalAnimShader.setFloat("bombLight.linear",    0.35f);
      skeletalAnimShader.setFloat("bombLight.quadratic", 0.44f);
      skeletalAnimShader.setVec3("bombLight.ambient",  bombLightIntensity * 0.3f, bombLightIntensity * 0.15f, 0.0f);
      skeletalAnimShader.setVec3("bombLight.diffuse",  bombLightIntensity * 1.0f, bombLightIntensity * 0.5f,  0.0f);
      skeletalAnimShader.setVec3("bombLight.specular", bombLightIntensity * 0.5f, bombLightIntensity * 0.25f, 0.0f);
    }

    // Send bones matrices
    auto transforms = animator.GetFinalBoneMatrices();
    for (size_t i = 0; i < transforms.size(); ++i)
      skeletalAnimShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

    if (currentCameraMode != MODE_FIRST_PERSON) {
      glm::mat4 model(1.0f);
      model = glm::translate(model, player_position) *
              glm::toMat4(bomberman.getOrientation());
      model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
      skeletalAnimShader.setMat4("model", model);
      skeletalAnimShader.setInt("isFireModel", 0);
      skeletalAnimShader.setFloat("shininess", 96.0f); // metal robot
      robot.Draw(skeletalAnimShader);
    }

    // Send bones matrices for the Fire
    transforms = fire_animator.GetFinalBoneMatrices();
    for (size_t i = 0; i < transforms.size(); ++i)
      skeletalAnimShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

    DrawFire(bomb, map, currentFrame, fire, skeletalAnimShader);

    // ENEMIES ------
    modelLoadingShader.use();

    // Send uniforms
    modelLoadingShader.setMat4("view", view);
    modelLoadingShader.setMat4("projection", projection);
    modelLoadingShader.setVec3("viewPos", camera.GetPosition());
    modelLoadingShader.setBool("bombLightActive", bombActive);

    if (bombActive) {
      glm::vec3 bpos = bomb.getBombPosition() + glm::vec3(0.0f, 0.5f, 0.0f);
      modelLoadingShader.setVec3("bombLight.position", bpos);
      modelLoadingShader.setFloat("bombLight.constant",  1.0f);
      modelLoadingShader.setFloat("bombLight.linear",    0.35f);
      modelLoadingShader.setFloat("bombLight.quadratic", 0.44f);
      modelLoadingShader.setVec3("bombLight.ambient",  bombLightIntensity * 0.3f, bombLightIntensity * 0.15f, 0.0f);
      modelLoadingShader.setVec3("bombLight.diffuse",  bombLightIntensity * 1.0f, bombLightIntensity * 0.5f,  0.0f);
      modelLoadingShader.setVec3("bombLight.specular", bombLightIntensity * 0.5f, bombLightIntensity * 0.25f, 0.0f);
    }

    // Fire point light (illuminates enemies and other objects)
    modelLoadingShader.setBool("fireLightActive", fireActive);
    if (fireActive) {
      glm::vec3 fpos = bomb.getBombPosition() + glm::vec3(0.0f, 0.3f, 0.0f);
      modelLoadingShader.setVec3("fireLight.position",  fpos);
      modelLoadingShader.setFloat("fireLight.constant",  1.0f);
      modelLoadingShader.setFloat("fireLight.linear",    0.14f);
      modelLoadingShader.setFloat("fireLight.quadratic", 0.07f);
      modelLoadingShader.setVec3("fireLight.ambient",   0.6f,  0.25f, 0.0f);
      modelLoadingShader.setVec3("fireLight.diffuse",   2.0f,  0.8f,  0.0f);
      modelLoadingShader.setVec3("fireLight.specular",  1.0f,  0.5f,  0.0f);
    }

    for (const auto &enemy : enemies) {
      glm::mat4 model(1.0f);
      
      float finalScale = 0.4f;
      float yOffset = 0.0f;

      if (enemy.getType() == EnemyType::BALLOM) {
          // Ballom breathing and up/down translation animation
          float inflationSpeed = 2.0f;
          float inflationIntensity = 0.05f;
          finalScale = enemy.getIsDead() ? enemy.getDeathScale() : (0.4f + sin(currentFrame * inflationSpeed) * inflationIntensity);
          
          float bobbingSpeed = 2.0f; 
          float bobbingIntensity = 0.15f;
          // Death animation
          float bobbing = enemy.getIsDead() ? 0.0f : abs(sin(currentFrame * bobbingSpeed)) * bobbingIntensity;
          
          yOffset = 0.55f + bobbing;
      } else {
          // Onil
          finalScale = enemy.getIsDead() ? enemy.getDeathScale() : 0.4f;
          yOffset = 0.3f;
      }
       
      glm::vec3 drawPosition = enemy.getPosition() + glm::vec3(0.0f, yOffset, 0.0f);
      
      model = glm::translate(model, drawPosition) * glm::toMat4(enemy.getOrientation()); 

      // Adjust scale as needed for the imported models
      model = glm::scale(model, glm::vec3(finalScale, finalScale, finalScale));
      
      // Fix predefined model orientation (y -90)
      model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
      
      modelLoadingShader.setMat4("model", model);
      if (enemy.getType() == EnemyType::BALLOM) {
        modelLoadingShader.setFloat("shininess", 64.0f); // latex/balloon — glossy
        ballomModel.Draw(modelLoadingShader);
      } else {
        modelLoadingShader.setFloat("shininess", 8.0f);  // Onil — matte
        // Dibujar el cuerpo
        onilCuerpoModel.Draw(modelLoadingShader);

        // Calcular la oscilacion de los pies
        float swingAngle = 0.0f;
        if (enemy.getIsMoving() && !enemy.getIsDead()) {
            float walkSpeed = 6.0f;
            float walkIntensity = 0.5f; // en radianes
            swingAngle = sin(currentFrame * walkSpeed) * walkIntensity;
        }

        // Pie Izquierdo
        glm::mat4 leftFootModel = model;
        leftFootModel = glm::rotate(leftFootModel, swingAngle, glm::vec3(0.0f, 0.0f, 1.0f));
        modelLoadingShader.setMat4("model", leftFootModel);
        onilPieIzqModel.Draw(modelLoadingShader);

        // Pie Derecho (fase opuesta, multiplicamos por -1)
        glm::mat4 rightFootModel = model;
        rightFootModel = glm::rotate(rightFootModel, -swingAngle, glm::vec3(0.0f, 0.0f, 1.0f));
        modelLoadingShader.setMat4("model", rightFootModel);
        onilPieDerModel.Draw(modelLoadingShader);
      }
    }

    glBindVertexArray(0);

    // Swap the screen buffers
    glfwSwapBuffers(window);
  }

  // Terminate GLFW, clearing any resources allocated by GLFW.
  glfwTerminate();

  return 0;
}

GLuint LoadTexture2D(const char *path) {
  // Load textures
  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                  GL_NEAREST_MIPMAP_NEAREST);

  // Diffuse map
  unsigned char *image;
  int textureWidth, textureHeight, nrChannels;
  image = stbi_load(path, &textureWidth, &textureHeight, &nrChannels, 0);

  if (!image) {
    std::cout << "Failed to load texture: " << path << std::endl;
    return 0;
  }

  GLenum format;

  // Chose the right format based on channels
  if (nrChannels == 1)
    format = GL_RED;
  else if (nrChannels == 3)
    format = GL_RGB;
  else if (nrChannels == 4)
    format = GL_RGBA;
  else {
    std::cout << "Unsupported channel count for: " << path << std::endl;
    stbi_image_free(image);
    return 0;
  }

  glTexImage2D(GL_TEXTURE_2D, 0, format, textureWidth, textureHeight, 0, format,
               GL_UNSIGNED_BYTE, image);
  glGenerateMipmap(GL_TEXTURE_2D);

  stbi_image_free(image);
  return texture;
}

GLuint CreateSolidTexture(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    GLuint texture;
    unsigned char color[] = { r, g, b, a };

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, color);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

void ApplyTexture(const Material& material, Shader& lightingShader) {
      // Bind diffuse map
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, material.diffuse);

      // Bind specular map
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, material.specular);

      lightingShader.setFloat("material.shininess", material.shininess);
      lightingShader.setVec2("uvScale", material.uvScaleX, material.uvScaleY);
  }

void DrawMap(const Map& map, Shader& lightingShader, const MapMaterials& map_materials) {
  // FLOOR ------
  DrawFloor(lightingShader, map_materials.ground_mat);

  // WALLS ------
  DrawWalls(lightingShader, map_materials.wall_mat);

  // MAP OBJECTS ------
  DrawMapBlocks(map, lightingShader, map_materials);
}

void DrawFloor(Shader& lightingShader, const Material& ground_mat) {
  glm::mat4 model(1.0f);

  ApplyTexture(ground_mat, lightingShader);

  // Model transformations
  model = glm::scale(model, glm::vec3(COLS, 1.0f, ROWS));
  model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
  lightingShader.setMat4("model", model);

  // Draw floor
  glDrawArrays(GL_TRIANGLES, 30, 6);
}

void DrawWalls(Shader& lightingShader, const Material& wall_mat) {
  glm::mat4 model(1.0f);

  ApplyTexture(wall_mat, lightingShader);

  for (float x = -half_cols; x <= half_cols; x += 1.0f) {
    model = glm::mat4(1.0f); // Reset model matrix
    model = glm::translate(model, glm::vec3(x, 0.0f, half_rows));
    lightingShader.setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    model = glm::mat4(1.0f); // Reset model matrix
    model = glm::translate(model, glm::vec3(x, 0.0f, -half_rows));
    lightingShader.setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
  }

  // Draw side walls
  for (float z = -half_rows + 1.0f; z <= half_rows - 1.0f; z += 1.0f) {
    // Reset model transformations
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(half_cols, 0.0f, z));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    lightingShader.setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Reset model transformations
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-half_cols, 0.0f, z));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    lightingShader.setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
  }
}

void DrawMapBlocks(const Map& map, Shader& lightingShader, const MapMaterials& map_materials) {
  glm::mat4 model(1.0f);

  for (int row = 0; row < ROWS; row++) {
    for (int col = 0; col < COLS; col++) {
      int cell = map.getCell(row, col);
      if (cell == 0 || cell == 5 || cell == 6)
        continue;

      float xPos = col - (half_cols - 1);
      float zPos = row - (half_rows - 1);

      if (cell == 1) {
        // Indestructible pillar
        ApplyTexture(map_materials.wall_mat, lightingShader);
      } else if (cell >= 2 && cell <= 4) {
        // Destructible brick (including hidden exit and power-ups)
        ApplyTexture(map_materials.brick_mat, lightingShader);
      }

      model = glm::mat4(1.0f);
      model = glm::translate(model, glm::vec3(xPos, 0.0f, zPos));
      lightingShader.setMat4("model", model);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }
  }
}

void DrawBomb(Bomb& bomb, Map& map, GLfloat currentTime, Shader& lightingShader, Model& bombModel, Shader& modelLoadingShader) {
    if (bomb.getBombState() == true) // If the bomb is activated
    {
      float blinkSpeed = 6.0f;
      float blinkIntensity = 0.05f;

      float blinkScale = sin(currentTime * blinkSpeed) * blinkIntensity;
      float fianlBlinkScale = 0.15f + (blinkScale * 0.5f); //default scale = 0.9f
      
      glm::vec3 bomb_position = bomb.getBombPosition();
      if (currentTime >= bomb.getBombExpiration()) { // If the bomb explodes
        bomb.expireBomb(bomb_position, map, currentTime);
      }

      modelLoadingShader.use();
      glm::mat4 model(1.0f);
      model = glm::translate(model, bomb_position);
      model = glm::scale(model, glm::vec3(fianlBlinkScale, fianlBlinkScale, fianlBlinkScale));
      modelLoadingShader.setMat4("model", model);
      bombModel.Draw(modelLoadingShader);
      lightingShader.use();
    }
}

void DrawFire(Bomb& bomb, Map& map, GLfloat currentTime, Model& fireModel, Shader& skeletalAnimShader) {

    if (bomb.isFireActive() == true) // If the bomb already exploded
    {
      glm::vec3 firePosition = bomb.getBombPosition() - glm::vec3(0.0f, 0.25f, 0.0f);
      MapIndices grid_position = map.toMapIndices(firePosition);

      if (currentTime >= bomb.getFireExpiration()) { // Fire end
        bomb.putOutFire(grid_position, map);
      }

      glm::vec3 center = firePosition + glm::vec3(0.0f, 0.45f, 0.0f); // elevated gradient center
      skeletalAnimShader.setInt("isFireModel", 1);

      // Center flame
      glm::mat4 model(1.0f);
      model = glm::translate(model, firePosition);
      model = glm::scale(model, glm::vec3(0.04f, 0.04f, 0.04f));
      skeletalAnimShader.setMat4("model", model);
      skeletalAnimShader.setVec3("fireCenter", center);
      fireModel.Draw(skeletalAnimShader);

      // Cross flames X axis
      {
        glm::vec3 posX = firePosition + glm::vec3( 1.0f, 0.0f, 0.0f);
        glm::vec3 negX = firePosition + glm::vec3(-1.0f, 0.0f, 0.0f);
        bool posXfree = map.getCell(map.toMapIndices(posX).row, map.toMapIndices(posX).col) == 6;
        bool negXfree = map.getCell(map.toMapIndices(negX).row, map.toMapIndices(negX).col) == 6;

        if (posXfree) {
          // Right half centered at +0.5 on X
          model = glm::mat4(1.0f);
          model = glm::translate(model, firePosition + glm::vec3(0.5f, 0.0f, 0.0f));
          model = glm::scale(model, glm::vec3(0.04f * 3.0f, 0.04f, 0.04f));
          skeletalAnimShader.setMat4("model", model);
          skeletalAnimShader.setVec3("fireCenter", center);
          fireModel.Draw(skeletalAnimShader);
        }
        if (negXfree) {
          // Left half centered at -0.5 on X
          model = glm::mat4(1.0f);
          model = glm::translate(model, firePosition + glm::vec3(-0.5f, 0.0f, 0.0f));
          model = glm::scale(model, glm::vec3(0.04f * 3.0f, 0.04f, 0.04f));
          skeletalAnimShader.setMat4("model", model);
          skeletalAnimShader.setVec3("fireCenter", center);
          fireModel.Draw(skeletalAnimShader);
        }
      }

      // Cross flames Z axis
      {
        glm::vec3 posZ = firePosition + glm::vec3(0.0f, 0.0f,  1.0f);
        glm::vec3 negZ = firePosition + glm::vec3(0.0f, 0.0f, -1.0f);
        bool posZfree = map.getCell(map.toMapIndices(posZ).row, map.toMapIndices(posZ).col) == 6;
        bool negZfree = map.getCell(map.toMapIndices(negZ).row, map.toMapIndices(negZ).col) == 6;

        if (posZfree) {
          model = glm::mat4(1.0f);
          model = glm::translate(model, firePosition + glm::vec3(0.0f, 0.0f, 0.5f));
          model = glm::scale(model, glm::vec3(0.04f, 0.04f, 0.04f * 3.0f));
          skeletalAnimShader.setMat4("model", model);
          skeletalAnimShader.setVec3("fireCenter", center);
          fireModel.Draw(skeletalAnimShader);
        }
        if (negZfree) {
          model = glm::mat4(1.0f);
          model = glm::translate(model, firePosition + glm::vec3(0.0f, 0.0f, -0.5f));
          model = glm::scale(model, glm::vec3(0.04f, 0.04f, 0.04f * 3.0f));
          skeletalAnimShader.setMat4("model", model);
          skeletalAnimShader.setVec3("fireCenter", center);
          fireModel.Draw(skeletalAnimShader);
        }
      }

      // Restore state
      skeletalAnimShader.setInt("isFireModel", 0);
    }
}

// Moves/alters the camera positions based on user input
void DoMovement(Player& bomberman, Map& map, std::vector<Enemy>& enemies, Bomb& bomb, Animator& animator, AnimationsSet& animations) {
  if (!playerIsAlive) { // Cant move if player dies
    // Restart when player dies
    if (keys[GLFW_KEY_R])
      restart(bomberman, map, enemies);
    else // Movement not allowed
      return;
  }

  // Camera controls
  if (currentCameraMode == MODE_FREE) {
    if (keys[GLFW_KEY_UP]) {
      camera.ProcessKeyboard(FORWARD, deltaTime);
    }

    if (keys[GLFW_KEY_DOWN]) {
      camera.ProcessKeyboard(BACKWARD, deltaTime);
    }

    if (keys[GLFW_KEY_LEFT]) {
      camera.ProcessKeyboard(LEFT, deltaTime);
    }

    if (keys[GLFW_KEY_RIGHT]) {
      camera.ProcessKeyboard(RIGHT, deltaTime);
    }
  }

  // Player controls
  if (currentCameraMode == MODE_FREE || currentCameraMode == MODE_SIDE_SCROLL) {
    moving = false;

    if (keys[GLFW_KEY_W]) {
      bomberman.ProcessKeyboard(NORTH, deltaTime, map);
      moving = true;
    }

    if (keys[GLFW_KEY_S]) {
      bomberman.ProcessKeyboard(SOUTH, deltaTime, map);
      moving = true;
    }

    if (keys[GLFW_KEY_A]) {
      bomberman.ProcessKeyboard(WEST, deltaTime, map);
      moving = true;
    }

    if (keys[GLFW_KEY_D]) {
      bomberman.ProcessKeyboard(EAST, deltaTime, map);
      moving = true;
    }

    SelectAnimation(animator, animations);

  } else if (currentCameraMode == MODE_FIRST_PERSON) {
    if (keys[GLFW_KEY_W]) {
      bomberman.ProcessKeyboardFPS(NORTH, camera.GetFront(), camera.GetRight(),
                                   deltaTime, map);
    }

    if (keys[GLFW_KEY_S]) {
      bomberman.ProcessKeyboardFPS(SOUTH, camera.GetFront(), camera.GetRight(),
                                   deltaTime, map);
    }

    if (keys[GLFW_KEY_A]) {
      bomberman.ProcessKeyboardFPS(WEST, camera.GetFront(), camera.GetRight(),
                                   deltaTime, map);
    }

    if (keys[GLFW_KEY_D]) {
      bomberman.ProcessKeyboardFPS(EAST, camera.GetFront(), camera.GetRight(),
                                   deltaTime, map);
    }
  }

  // Place bomb
  bool can_place_bomb = bomb.getBombState() == false && bomb.isFireActive() == false;
  if (keys[GLFW_KEY_SPACE] && can_place_bomb) {
    bomb.activateBomb(bomberman.getPosition(), glfwGetTime(), map);
    bomberman.setCanPassBomb(true);
    animator.PlayAnimation(&animations.punchAnim, false);
  }
}

void SelectAnimation(Animator& animator, AnimationsSet& animations) {
  if(!animator.IsLooping())
  {
    if(!animator.IsFinished() || !playerIsAlive)
      return;
    else
    {
      animator.PlayAnimation(&animations.idleAnim, true);
    }
  }
  if (moving)
  {
    if (animator.GetCurrentAnimation() != &animations.walkingAnim)
    {
      animator.PlayAnimation(&animations.walkingAnim, true);
    }
  }
  else
  {
    if (animator.GetCurrentAnimation() != &animations.idleAnim)
    {
      animator.PlayAnimation(&animations.idleAnim, true);
    }
  }
}

void restart(Player& bomberman, Map& map, std::vector<Enemy>& enemies) {
  // Regen map
  map.genMap();
  map.genHidden();
  // Restart enemies
  enemies.clear();
  enemies = Enemy::SpawnEnemies(map, 6, 8);

  // Reset the player
  playerIsAlive = true;
  bomberman.resetTo(glm::vec3(-half_cols + 1, -0.49f, -half_rows + 1), glm::vec3(0.0f, 0.0f, 1.0f));
}

// Is called whenever a key is pressed/released via GLFW
void KeyCallback(GLFWwindow *window, int key, int scancode, int action,
                 int mode) {
  if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action) {
    glfwSetWindowShouldClose(window, GL_TRUE);
  }

  if (key == GLFW_KEY_C && action == GLFW_PRESS) {
    if (currentCameraMode == MODE_SIDE_SCROLL) currentCameraMode = MODE_FIRST_PERSON;
    else if (currentCameraMode == MODE_FIRST_PERSON) currentCameraMode = MODE_FREE;
    else currentCameraMode = MODE_SIDE_SCROLL;
  }

  if (key >= 0 && key < 1024) {
    if (action == GLFW_PRESS) {
      keys[key] = true;
    } else if (action == GLFW_RELEASE) {
      keys[key] = false;
    }
  }
}

void MouseCallback(GLFWwindow *window, double xPos, double yPos) {
  if (firstMouse) {
    lastX = xPos;
    lastY = yPos;
    firstMouse = false;
  }

  GLfloat xOffset = xPos - lastX;
  GLfloat yOffset =
      lastY - yPos; // Reversed since y-coordinates go from bottom to left

  lastX = xPos;
  lastY = yPos;

  camera.ProcessMouseMovement(xOffset, yOffset);
}
