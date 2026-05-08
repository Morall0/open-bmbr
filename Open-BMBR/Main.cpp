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
#include "animation.h"
#include "animator.h"
#include "Bomb.hpp"
#include "Camera.h"
#include "Map.hpp"
// #include "Model.h"
#include "model_animation.h"
#include "Player.hpp"
#include "Enemy.hpp"
#include "Shader.h"

// Structure for Map Drawing functions
struct MapAssets {
    GLuint groundTexture;
    GLuint groundSpecular;
    GLuint wallTexture;
    GLuint brickTexture;
};

// Function prototypes
GLuint LoadTexture2D(const char *path);
void KeyCallback(GLFWwindow *window, int key, int scancode, int action,
                 int mode);
void MouseCallback(GLFWwindow *window, double xPos, double yPos);
void DoMovement(Player& bomberman, Map& map, Bomb& bomb);

// Map Draw Functions
void DrawMap(const Map& map, Shader& lightingShader, const MapAssets& assets);
void DrawFloor(Shader& lightingShader, const MapAssets& assets);
void DrawWalls(Shader& lightingShader, GLuint wallTexture);
void DrawMapBlocks(const Map& map, Shader& lightingShader, const MapAssets& assets);

// Bomb Draw Function
void DrawBomb(Bomb& bomb, GLuint bomb_texture, Map& map, GLfloat currentTime, Shader& lightingShader);

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true;

// Map dimensions
const int ROWS = 11, COLS = 29;

const float half_rows =(ROWS + 1)/2.0f;
const float half_cols =(COLS + 1)/2.0f;

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
    std::cout << "Failed to initialize GLEW" << std::endl; return EXIT_FAILURE;
  }

  // OpenGL options
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);

  // Define the viewport dimensions
  glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

  // SHADERS
  Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");
  Shader skeletalAnimShader("Shader/modelLoading.vs", "Shader/modelLoading.frag");

  // MODEL LOADING
  Model robot("Models/Robot.gltf");
  // Animation robotAnimation("Models/Robot.glb", &robot);
  // Animator animator(&robotAnimation);

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

  // Load textures
  stbi_set_flip_vertically_on_load(true);
  GLuint ground_texture = LoadTexture2D("images/aerial_rocks_02_diff_1k.png");
  GLuint ground_specular = LoadTexture2D("images/aerial_rocks_02_rough_1k.png");
  GLuint wall_texture = LoadTexture2D("images/native_wall_1.png");

  // Configuring lightingShader
  lightingShader.use();

  // Material properties
  lightingShader.setInt("material.diffuse", 0);
  lightingShader.setInt("material.specular", 1);
  lightingShader.setFloat("material.shininess", 16.0f);

  // Dir light properties
  lightingShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
  lightingShader.setVec3("dirLight.ambient", 0.3f, 0.3f, 0.3f);
  lightingShader.setVec3("dirLight.diffuse", 0.3f, 0.3f, 0.3f);
  lightingShader.setVec3("dirLight.specular", 0.0f, 0.0f, 0.0f);

  // Initialize random seed
  srand(static_cast<unsigned int>(time(NULL)));

  // Generate a dark gray texture for destructible bricks
  GLuint brick_texture;
  glGenTextures(1, &brick_texture);
  glBindTexture(GL_TEXTURE_2D, brick_texture);
  unsigned char darkGray[] = {64, 64, 64, 255};
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, darkGray);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  // Generate a red texture for enemies
  GLuint enemy_red_texture;
  glGenTextures(1, &enemy_red_texture);
  glBindTexture(GL_TEXTURE_2D, enemy_red_texture);
  unsigned char redColor[] = {255, 0, 0, 255};
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, redColor);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  // Generate a blue texture for Onil enemies
  GLuint enemy_blue_texture;
  glGenTextures(1, &enemy_blue_texture);
  glBindTexture(GL_TEXTURE_2D, enemy_blue_texture);
  unsigned char blueColor[] = {0, 0, 255, 255};
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, blueColor);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  // Generate a red texture for enemies
  GLuint bomb_texture;
  glGenTextures(1, &bomb_texture);
  glBindTexture(GL_TEXTURE_2D, bomb_texture);
  unsigned char black_color[] = {255, 255, 255, 255};
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, black_color);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  // Map
  // Create map logic
  Map map(ROWS, COLS);
  map.genMap();
  map.genHidden();
  // map.printMap(); // Used to print map to console for debugging

  // Spawning enemies
  std::vector<Enemy> enemies = Enemy::SpawnEnemies(map, 6, 8);

  // Set map assets for drawing
  MapAssets mapAssets;
  mapAssets.groundTexture = ground_texture;
  mapAssets.groundSpecular = ground_specular;
  mapAssets.wallTexture = wall_texture;
  mapAssets.brickTexture = brick_texture;

  // Player
  // The initial position is in the secure zone map(0,0)
  Player bomberman(glm::vec3(-half_cols + 1, -0.49f, -half_rows + 1), glm::vec2(0.0f, 1.0f));

  // Initializing Bomb object with duratin 3s and -0.1 y_pos
  Bomb bomb(3.0f, -0.1f);

  // Set the projection type and parameters
  glm::mat4 projection = glm::perspective(camera.GetZoom(), 
                                          (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 
                                          0.1f,
                                          100.0f);

  // Game loop
  while (!glfwWindowShouldClose(window)) {
    // Calculate deltatime of current frame
    GLfloat currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // Check if any events have been activated (key pressed, mouse moved etc.)
    // and call corresponding response functions
    glfwPollEvents();
    DoMovement(bomberman, map, bomb);

    for (auto& enemy : enemies) {
      enemy.Update(deltaTime, map, bomberman.getPosition());
    }

    // Update Animation
    // animator.UpdateAnimation(deltaTime);

    // Clear the colorbuffer
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    lightingShader.use();

    // CAMERA ------
    glm::mat4 view = camera.GetViewMatrix();
    lightingShader.setMat4("view", view);
    lightingShader.setMat4("projection", projection); // Set projection
    lightingShader.setVec3("viewPos", camera.GetPosition());

    glBindVertexArray(VAO);

    // MAP ------
    DrawMap(map, lightingShader, mapAssets);

    // ENEMIES ------
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0); // No specular

    lightingShader.setVec2("uvScale", 1.0f, 1.0f);

    for (const auto& enemy : enemies) {
        glActiveTexture(GL_TEXTURE0);
        if (enemy.getType() == EnemyType::BALLOM) {
            glBindTexture(GL_TEXTURE_2D, enemy_red_texture);
        } else {
            glBindTexture(GL_TEXTURE_2D, enemy_blue_texture);
        }

        glm::mat4 model(1.0f);
        // Translate to enemy position and apply orientation (though a simple cube doesn't show orientation well, we'll add it)
        model = glm::translate(model, enemy.getPosition()) * glm::toMat4(enemy.getOrientation());
        model = glm::scale(model, glm::vec3(0.8f, 0.8f, 0.8f)); // A bit smaller than the walls
        lightingShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // BOMBS ------
    DrawBomb(bomb, bomb_texture, map, currentFrame, lightingShader);

    // ROBOT ------
    skeletalAnimShader.use();
    
    // Send uniforms
    skeletalAnimShader.setMat4("view", view);
    skeletalAnimShader.setMat4("projection", projection);

    // Send bones matrices
    // auto transforms = animator.GetFinalBoneMatrices();
    // for (size_t i = 0; i < transforms.size(); ++i)
    //   skeletalAnimShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

    glm::mat4 model(1.0f);
    model = glm::translate(model, bomberman.getPosition()) * glm::toMat4(bomberman.getOrientation());
    model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
    skeletalAnimShader.setMat4("model", model);
    robot.Draw(skeletalAnimShader);

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
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);

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

  glTexImage2D(GL_TEXTURE_2D, 0, format, textureWidth, textureHeight, 0, format, GL_UNSIGNED_BYTE, image);
  glGenerateMipmap(GL_TEXTURE_2D);

  stbi_image_free(image);
  return texture;
}


void DrawMap(const Map& map, Shader& lightingShader, const MapAssets& assets) {
  // FLOOR ------
  DrawFloor(lightingShader, assets);

  // WALLS ------
  DrawWalls(lightingShader, assets.wallTexture);

  // MAP OBJECTS ------
  DrawMapBlocks(map, lightingShader, assets);

}

void DrawFloor(Shader& lightingShader, const MapAssets& assets) {
  glm::mat4 model(1.0f);

  // Bind diffuse map
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, assets.groundTexture);
  // Bind specular map
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, assets.groundSpecular);

  lightingShader.setVec2("uvScale", 10.33f, 4.33f); // texture scale

  // Model transformations
  model = glm::scale(model, glm::vec3(COLS, 1.0f, ROWS));
  model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
  lightingShader.setMat4("model", model);

  // Draw floor
  glDrawArrays(GL_TRIANGLES, 30, 6);
}

void DrawWalls(Shader& lightingShader, GLuint wallTexture) {
  glm::mat4 model(1.0f);

  // Bind diffuse map
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, wallTexture);
  // Unbind specular map for walls so they don't use the floor's specular
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, 0);

  lightingShader.setVec2("uvScale", 1.0f, 1.0f); // texture scale

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
  for (float z = -half_rows+1.0f; z <= half_rows-1.0f; z += 1.0f) {
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

void DrawMapBlocks(const Map& map, Shader& lightingShader, const MapAssets& assets) {
  glm::mat4 model(1.0f);

  for (int row = 0; row < ROWS; row++) {
    for (int col = 0; col < COLS; col++) {
      int cell = map.getCell(row, col);
      if (cell == 0 || cell == 5)
        continue;

      float xPos = col - (half_cols - 1);
      float zPos = row - (half_rows - 1);

      if (cell == 1) {
        // Indestructible pillar
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, assets.wallTexture);
      } else if (cell >= 2 && cell <= 4) {
        // Destructible brick (including hidden exit and power-ups)
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, assets.brickTexture);
      }

      model = glm::mat4(1.0f);
      model = glm::translate(model, glm::vec3(xPos, 0.0f, zPos));
      lightingShader.setMat4("model", model);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }
  }
}

void DrawBomb(Bomb& bomb, GLuint bomb_texture, Map& map, GLfloat currentTime, Shader& lightingShader) {
    if (bomb.getBombState() == true) // If the bomb is activated
    {
      if (currentTime >= bomb.getBombExpiration()) // If the bomb explodes
        bomb.expireBomb(bomb.getBombPosition(), map);

      // Texture
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, bomb_texture);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, 0); // No specular

      lightingShader.setVec2("uvScale", 1.0f, 1.0f);

      glm::mat4 model(1.0f);
      model = glm::translate(model, bomb.getBombPosition());
      model = glm::scale(model, glm::vec3(0.9f, 0.9f, 0.9f));
      lightingShader.setMat4("model", model);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}

// Moves/alters the camera positions based on user input
void DoMovement(Player& bomberman, Map& map, Bomb& bomb) {
  // Camera controls
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

  // Player controls
  if (keys[GLFW_KEY_W]) {
    bomberman.ProcessKeyboard(NORTH, deltaTime, map);
  }

  if (keys[GLFW_KEY_S]) {
    bomberman.ProcessKeyboard(SOUTH, deltaTime, map);
  }

  if (keys[GLFW_KEY_A]) {
    bomberman.ProcessKeyboard(WEST, deltaTime, map);
  }

  if (keys[GLFW_KEY_D]) {
    bomberman.ProcessKeyboard(EAST, deltaTime, map);
  }

  // Place bomb
  if (keys[GLFW_KEY_SPACE] && bomb.getBombState() == false) { 
    bomb.activateBomb(bomberman.getPosition(), glfwGetTime(), map);
    bomberman.setCanPassBomb(true);
  }
}

// Is called whenever a key is pressed/released via GLFW
void KeyCallback(GLFWwindow *window, int key, int scancode, int action,
                 int mode) {
  if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action) {
    glfwSetWindowShouldClose(window, GL_TRUE);
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
