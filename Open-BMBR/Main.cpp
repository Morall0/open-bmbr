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

#include "Drawer.hpp"

struct AnimationsSet {
  Animation winAnim;
  Animation idleAnim;
  Animation walkingAnim;
  Animation deadAnim;
  Animation punchAnim;
};

// Function prototypes
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow *window, double xPos, double yPos);
void DoMovement(Player& bomberman, Map& map, std::vector<Enemy>& enemies, std::vector<Bomb>& bombs, Animator& animator, AnimationsSet& animations);

void SelectAnimation(Animator& animator, AnimationsSet& animations);

// Function to reset the game
void restart(Player& bomberman, Map& map, std::vector<Enemy>& enemies);

// Window dimensions
const GLuint WIDTH = 1200, HEIGHT = 800;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true;

// Camera modes
enum CameraMode {
  MODE_FREE,
  MODE_FIRST_PERSON,
  MODE_SIDE_SCROLL
};

// Initialize the camera mode in SCROLL by default
CameraMode currentCameraMode = MODE_SIDE_SCROLL;

// Map dimensions
const int ROWS = 11, COLS = 29;
const float half_rows = (ROWS + 1) / 2.0f;
const float half_cols = (COLS + 1) / 2.0f;

// Player state flags
bool moving = false;
bool playerIsAlive = true;
bool playerWon = false;

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
    Animation("Models/Robot.gltf", &robot, 0),
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

  Model doorModel("Models/exit_shield.fbx");
  Animation door_anim("Models/exit_shield.fbx", &doorModel, 0);
  Animator door_animator(&door_anim, true);

  Model doorBaseModel("Models/exit_base.fbx");

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

  // Instantiate Drawer and initialize materials
  Drawer renderer(lightingShader, skeletalAnimShader, modelLoadingShader);
  renderer.InitMapMaterials();

  // Configuring lightingShader
  lightingShader.use();

  // Material properties
  lightingShader.setInt("material.diffuse", 0);
  lightingShader.setInt("material.specular", 1);
  lightingShader.setInt("material.ao", 2);

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
  map.printMap(); // Used to print map to console for debugging

  // Spawning enemies
  std::vector<Enemy> enemies = Enemy::SpawnEnemies(map, 3, 5);

  // Player
  // The initial position is in the secure zone map(0,0)
  Player bomberman(glm::vec3(-half_cols + 1, -0.49f, -half_rows + 1), glm::vec2(0.0f, 1.0f));

  // Vector for active bombs
  std::vector<Bomb> activeBombs;

  // Set the projection type and parameters
  glm::mat4 projection = glm::perspective(camera.GetZoom(), 
                                          (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 
                                          0.1f,
                                          100.0f);

  // Previous mode to control the camera change
  CameraMode prevCameraMode = currentCameraMode;

  // Game loop
  while (!glfwWindowShouldClose(window)) {
    // Calculate deltatime of current frame
    GLfloat currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // Check if any events have been activated (key pressed, mouse moved etc.)
    glfwPollEvents();
    DoMovement(bomberman, map, enemies, activeBombs, animator, animations);

    bomberman.collectPowerups(map);

    glm::vec3 player_position = bomberman.getPosition();

    // Remove expired bombs
    for (auto it = activeBombs.begin(); it != activeBombs.end(); ) {
      if (!it->getBombState() && !it->isFireActive()) {
        it = activeBombs.erase(it);
      } else {
        ++it;
      }
    }

    // Enemies update and remove when dead
    for (auto it = enemies.begin(); it != enemies.end(); ) {
      it->Update(deltaTime, map, player_position);
      // Delete from vector only when dead animation ends (scale 0)
      if (it->getIsDead() && it->getDeathScale() <= 0.0f) {
        it = enemies.erase(it);
      } else {
        ++it;
      }
    }

    // Update keyframe animations
    animator.UpdateAnimation(deltaTime);
    fire_animator.UpdateAnimation(deltaTime);
    door_animator.UpdateAnimation(deltaTime);

    // Clear the colorbuffer
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    lightingShader.use();

    // Detect camera mode transition
    if (currentCameraMode != prevCameraMode) {
      if (currentCameraMode == MODE_FREE) {
        camera.setPosition(glm::vec3(0.0f, 11.0f, 7.8f)); // Starts in pseudo-isometric position
        camera.setYawPitch(-90.0f, -60.0f);
      } else if (currentCameraMode == MODE_FIRST_PERSON) {
        // Sync yaw to current player orientation
        glm::vec3 playerDir = bomberman.getOrientation() * glm::vec3(0.0f, 0.0f, 1.0f);
        GLfloat targetYaw = glm::degrees(atan2(playerDir.z, playerDir.x));
        
        camera.setYawPitch(targetYaw, 0.0f);
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

    // UPDATE LIGHTS ------
    // Updates dynamic light positions and intensities for all shaders
    renderer.SetupLights(activeBombs, currentFrame);

    // MAP ------
    // Draws the static map grid including floors, walls, and pillars
    renderer.DrawMap(map, currentFrame);

    // BOMBS ------
    // Renders the bomb model with an accelerating blinking effect
    renderer.DrawBomb(activeBombs, map, currentFrame, bombModel);

    // DOOR ------
    // Draws the exit hatch when it is revealed on the map
    if (map.hasDoorRevealed()) {
      bool locked = !enemies.empty();
      renderer.DrawDoor(map, doorBaseModel, doorModel, door_animator, locked);
    }

    // Check if the player dies from fire
    bool fireCollision = false;
    for (auto& bomb : activeBombs) {
      if (bomb.isFireActive() && bomb.checkCollision(player_position, map)) {
        fireCollision = true;
        break;
      }
    }

    if (fireCollision && playerIsAlive) {
      playerIsAlive = false;
      animator.PlayAnimation(&animations.deadAnim, false);
    }

    // Check if the player wins (stands on the revealed door with no enemies alive)
    if (playerIsAlive && map.hasDoorRevealed() && enemies.empty()) {
      MapIndices doorIdx = map.getDoorPosition();
      MapIndices playerIdx = map.toMapIndices(player_position);
      if (playerIdx.row == doorIdx.row && playerIdx.col == doorIdx.col) {
        if (!playerWon) {
          std::cout << "\n=====================================" << std::endl;
          std::cout << "   VICTORY! All enemies defeated!    " << std::endl;
          std::cout << "     Press 'R' to play again         " << std::endl;
          std::cout << "=====================================\n" << std::endl;
          playerWon = true;
          animator.PlayAnimation(&animations.winAnim, true);
        }
      }
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
    // Renders the main player model and handles its bone animations
    skeletalAnimShader.use();

    // Send uniforms
    skeletalAnimShader.setMat4("view", view);
    skeletalAnimShader.setMat4("projection", projection);
    skeletalAnimShader.setVec3("viewPos", camera.GetPosition());

    renderer.DrawRobot(bomberman, robot, animator, currentCameraMode == MODE_FIRST_PERSON);

    // FIRE ------
    // Renders the cross-shaped fire explosion particles
    renderer.DrawFire(activeBombs, map, currentFrame, fire, fire_animator);

    // ENEMIES ------
    // Draws all enemies with their respective movement and death animations
    modelLoadingShader.use();

    // Send uniforms
    modelLoadingShader.setMat4("view", view);
    modelLoadingShader.setMat4("projection", projection);
    modelLoadingShader.setVec3("viewPos", camera.GetPosition());

    renderer.DrawEnemies(enemies, currentFrame, ballomModel, onilCuerpoModel, onilPieIzqModel, onilPieDerModel);

    glBindVertexArray(0);

    // Swap the screen buffers
    glfwSwapBuffers(window);
  }

  // Terminate GLFW, clearing any resources allocated by GLFW.
  glfwTerminate();

  return 0;
}

// Moves/alters the camera positions based on user input
void DoMovement(Player& bomberman, Map& map, std::vector<Enemy>& enemies, std::vector<Bomb>& bombs, Animator& animator, AnimationsSet& animations) {
  if (!playerIsAlive || playerWon) { // Cant move if player dies or wins
    // Restart when player dies or wins
    if (keys[GLFW_KEY_R])
    {
      restart(bomberman, map, enemies);
      map.printMap();
    }
    else // Movement not allowed
      return;
  }

  // Camera controls
  if (currentCameraMode == MODE_FREE) {
    if (keys[GLFW_KEY_UP]) {
      camera.ProcessKeyboard(FORWARD, deltaTime, half_cols, half_rows);
    }

    if (keys[GLFW_KEY_DOWN]) {
      camera.ProcessKeyboard(BACKWARD, deltaTime, half_cols, half_rows);
    }

    if (keys[GLFW_KEY_LEFT]) {
      camera.ProcessKeyboard(LEFT, deltaTime, half_cols, half_rows);
    }

    if (keys[GLFW_KEY_RIGHT]) {
      camera.ProcessKeyboard(RIGHT, deltaTime, half_cols, half_rows);
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
  int active_count = 0;
  for (auto& b : bombs) {
    if (b.getBombState() || b.isFireActive()) active_count++;
  }
  bool can_place_bomb = (active_count < bomberman.getMaxBombs());

  if (keys[GLFW_KEY_SPACE] && can_place_bomb) {
    MapIndices p_idx = map.toMapIndices(bomberman.getPosition());
    int cell = map.getCell(p_idx.row, p_idx.col);
    if (cell != 7 && cell != 8 && cell != 11 && cell != 12 && cell != 5) { // Do not overwrite door, powerups, or other bombs
      Bomb newBomb(2.5f, -0.2f);
      newBomb.activateBomb(bomberman.getPosition(), glfwGetTime(), map, bomberman.getFireRadius());
      bombs.push_back(newBomb);
      bomberman.setCanPassBomb(true);
      animator.PlayAnimation(&animations.punchAnim, false);
      keys[GLFW_KEY_SPACE] = false; // Prevent holding space to spam bombs
    }
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
  playerWon = false;
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
