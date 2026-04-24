#include <cmath>
#include <cstdlib>
#include <ctime>
#include <glm/ext/matrix_transform.hpp>
#include <iostream>
#include <vector>

// GLEW
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Other Libs
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Load Models
#include "SOIL2/SOIL2.h"

// Other includes
#include "Camera.h"
#include "Model.h"
#include "Shader.h"

// Function prototypes
GLuint LoadTexture2D(const char *path);
void KeyCallback(GLFWwindow *window, int key, int scancode, int action,
                 int mode);
void MouseCallback(GLFWwindow *window, double xPos, double yPos);
void DoMovement();

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true;

float vertices[] = {
    // Vertex coords     // Normal cords      // Texcoords
    -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 1.0f,  0.0f, // Back
    0.5f,  -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  0.5f,  0.5f,
    -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f,  1.0f,  0.5f,  0.5f,  -0.5f, 0.0f,
    0.0f,  -1.0f, 0.0f,  1.0f,  -0.5f, 0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f,
    1.0f,  1.0f,  -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 1.0f,  0.0f,

    -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f, // Front
    0.5f,  -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,  0.5f,  0.5f,
    0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,  0.5f,  0.5f,  0.5f,  0.0f,
    0.0f,  1.0f,  1.0f,  1.0f,  -0.5f, 0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    0.0f,  1.0f,  -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

    -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  1.0f,  1.0f, // Left
    -0.5f, 0.5f,  -0.5f, -1.0f, 0.0f,  0.0f,  0.0f,  1.0f,  -0.5f, -0.5f,
    -0.5f, -1.0f, 0.0f,  0.0f,  0.0f,  0.0f,  -0.5f, -0.5f, -0.5f, -1.0f,
    0.0f,  0.0f,  0.0f,  0.0f,  -0.5f, -0.5f, 0.5f,  -1.0f, 0.0f,  0.0f,
    1.0f,  0.0f,  -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  1.0f,  1.0f,

    0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f, // Right
    0.5f,  0.5f,  -0.5f, 1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  0.5f,  -0.5f,
    -0.5f, 1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.5f,  -0.5f, -0.5f, 1.0f,
    0.0f,  0.0f,  1.0f,  0.0f,  0.5f,  -0.5f, 0.5f,  1.0f,  0.0f,  0.0f,
    0.0f,  0.0f,  0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,

    -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.0f,  0.0f, // Bottom
    0.5f,  -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  1.0f,  0.0f,  0.5f,  -0.5f,
    0.5f,  0.0f,  -1.0f, 0.0f,  1.0f,  1.0f,  0.5f,  -0.5f, 0.5f,  0.0f,
    -1.0f, 0.0f,  1.0f,  1.0f,  -0.5f, -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,
    0.0f,  1.0f,  -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.0f,  0.0f,

    -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.0f,  1.0f, // Up
    0.5f,  0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  1.0f,  1.0f,  0.5f,  0.5f,
    0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  0.0f,
    1.0f,  0.0f,  1.0f,  0.0f,  -0.5f, 0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    0.0f,  0.0f,  -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.0f,  1.0f};

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
      glfwCreateWindow(WIDTH, HEIGHT, "Fuentes de luz", nullptr, nullptr);

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

  // MODEL LOADING
  // Model model((char*)"Models/model_name.obj");

  // First, set the container's VAO (and VBO)
  GLuint VBO, VAO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  // Position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                        (GLvoid *)0);
  glEnableVertexAttribArray(0);
  // normal attribute
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  // Texture Coordinate attribute
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                        (GLvoid *)(6 * sizeof(GLfloat)));
  glEnableVertexAttribArray(2);
  glBindVertexArray(0);

  // Load textures
  stbi_set_flip_vertically_on_load(true);
  GLuint ground_texture = LoadTexture2D("images/aerial_rocks_02_diff_1k.png");
  GLuint ground_specular = LoadTexture2D("images/aerial_rocks_02_rough_1k.png");
  GLuint wall_texture = LoadTexture2D("images/native_wall_1.png");

  // Use cooresponding shader and set uniforms
  lightingShader.Use();

  // Get the uniform locations
  GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
  GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
  GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");

  GLint viewPosLoc = glGetUniformLocation(
      lightingShader.Program, "viewPos"); // Uniform for camera position

  GLint uvScaleLoc = glGetUniformLocation(
      lightingShader.Program, "uvScale"); // Uniform for texture scaling

  // Material properties
  glUniform1i(glGetUniformLocation(lightingShader.Program, "material.diffuse"),
              0);
  glUniform1i(glGetUniformLocation(lightingShader.Program, "material.specular"),
              1);
  glUniform1f(
      glGetUniformLocation(lightingShader.Program, "material.shininess"),
      16.0f);

  // Dir light properties
  glUniform3f(
      glGetUniformLocation(lightingShader.Program, "dirLight.direction"), -0.2f,
      -1.0f, -0.3f);
  glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"),
              0.3f, 0.3f, 0.3f);
  glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"),
              0.3f, 0.3f, 0.3f);
  glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"),
              0.0f, 0.0f, 0.0f);

  // Initialize random seed
  srand(static_cast<unsigned int>(time(NULL)));

  // Generate a dark gray texture for destructible bricks
  GLuint brick_texture;
  glGenTextures(1, &brick_texture);
  glBindTexture(GL_TEXTURE_2D, brick_texture);
  unsigned char darkGray[] = {64, 64, 64, 255};
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               darkGray);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  // Map Generation (11 rows, 29 cols)
  int mapGrid[11][29] = {0};
  std::vector<std::pair<int, int>> destructibleBricks;

  for (int row = 0; row < 11; row++) {
    for (int col = 0; col < 29; col++) {
      // Safe zone at top-left
      if ((row == 0 && col == 0) || (row == 0 && col == 1) ||
          (row == 1 && col == 0)) {
        mapGrid[row][col] = 0;
        continue;
      }

      if (row % 2 == 1 && col % 2 == 1) {
        mapGrid[row][col] = 1; // Indestructible pillar
      } else {
        // 60% chance of destructible brick
        if (rand() % 100 < 60) {
          mapGrid[row][col] = 2; // Destructible brick
          destructibleBricks.push_back(std::make_pair(row, col));
        }
      }
    }
  }

  // Generate hidden items inside the destructible bricks
  if (!destructibleBricks.empty()) {
    // 1. Exit Door (represented as 3)
    int exitIndex = rand() % destructibleBricks.size();
    mapGrid[destructibleBricks[exitIndex].first]
           [destructibleBricks[exitIndex].second] = 3;
    destructibleBricks.erase(destructibleBricks.begin() + exitIndex);
  }

  if (!destructibleBricks.empty()) {
    // 2. Power-Ups (represented as 4), randomly 1 or 2
    int numPowerUps = (rand() % 2) + 1;
    for (int i = 0; i < numPowerUps && !destructibleBricks.empty(); i++) {
      int powerIndex = rand() % destructibleBricks.size();
      mapGrid[destructibleBricks[powerIndex].first]
             [destructibleBricks[powerIndex].second] = 4;
      destructibleBricks.erase(destructibleBricks.begin() + powerIndex);
    }
  }

  // Print map to console for verification
  std::cout << "\n--- MAPA GENERADO ---" << std::endl;
  for (int row = 0; row < 11; row++) {
    for (int col = 0; col < 29; col++) {
      if (mapGrid[row][col] == 0)
        std::cout << "  "; // Empty
      else if (mapGrid[row][col] == 1)
        std::cout << "[]"; // Pillar
      else if (mapGrid[row][col] == 2)
        std::cout << "##"; // Brick
      else if (mapGrid[row][col] == 3)
        std::cout << "EE"; // Exit Door
      else if (mapGrid[row][col] == 4)
        std::cout << "PP"; // Power-up
    }
    std::cout << std::endl;
  }
  std::cout << "---------------------\n" << std::endl;

  // Set the projection type and parameters
  glm::mat4 projection = glm::perspective(
      camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f,
      100.0f);

  glUniformMatrix4fv(projLoc, 1, GL_FALSE,
                     glm::value_ptr(projection)); // Pass the projection matrix

  // Game loop
  while (!glfwWindowShouldClose(window)) {
    // Calculate deltatime of current frame
    GLfloat currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // Check if any events have been activated (key pressed, mouse moved etc.)
    // and call corresponding response functions
    glfwPollEvents();
    DoMovement();

    // Clear the colorbuffer
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    lightingShader.Use();

    // CAMERA ------
    glm::mat4 view = camera.GetViewMatrix();

    // Pass view matrix
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // Pass camera position
    glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y,
                camera.GetPosition().z);

    glm::mat4 model(1.0f);

    // FLOOR ------
    // Bind diffuse map
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ground_texture);
    // Bind specular map
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, ground_specular);

    glUniform2f(uvScaleLoc, 10.33f, 4.33f); // texture scale

    // Model transformations
    model = glm::scale(model, glm::vec3(31.0f, 1.0f, 13.0f));
    model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // Draw floor
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 30, 6);

    // WALLS ------
    // Bind diffuse map
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, wall_texture);
    // Unbind specular map for walls so they don't use the floor's specular
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);

    glUniform2f(uvScaleLoc, 1.0f, 1.0f); // Pass texture scale

    // Draw front and back walls
    for (float x = -15.0f; x <= 15.0f; x += 1.0f) {
      model = glm::mat4(1.0f); // Reset model matrix
      model = glm::translate(model, glm::vec3(x, 0.0f, 6.0f));
      glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
      glDrawArrays(GL_TRIANGLES, 0, 36);

      model = glm::mat4(1.0f); // Reset model matrix
      model = glm::translate(model, glm::vec3(x, 0.0f, -6.0f));
      glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // Draw side walls
    for (float z = -5.0f; z <= 5.0f; z += 1.0f) {
      // Reset model transformations
      model = glm::mat4(1.0f);
      model = glm::translate(model, glm::vec3(15.0f, 0.0f, z));
      model =
          glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
      glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
      glDrawArrays(GL_TRIANGLES, 0, 36);

      // Reset model transformations
      model = glm::mat4(1.0f);
      model = glm::translate(model, glm::vec3(-15.0f, 0.0f, z));
      model =
          glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
      glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // MAP OBJECTS ------
    for (int row = 0; row < 11; row++) {
      for (int col = 0; col < 29; col++) {
        int cell = mapGrid[row][col];
        if (cell == 0)
          continue;

        float xPos = col - 14.0f;
        float zPos = row - 5.0f;

        if (cell == 1) {
          // Indestructible pillar
          glActiveTexture(GL_TEXTURE0);
          glBindTexture(GL_TEXTURE_2D, wall_texture);
        } else if (cell >= 2 && cell <= 4) {
          // Destructible brick (including hidden exit and power-ups)
          glActiveTexture(GL_TEXTURE0);
          glBindTexture(GL_TEXTURE_2D, brick_texture);
        }

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(xPos, 0.0f, zPos));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);
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

// Moves/alters the camera positions based on user input
void DoMovement() {
  // Camera controls
  if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP]) {
    camera.ProcessKeyboard(FORWARD, deltaTime);
  }

  if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN]) {
    camera.ProcessKeyboard(BACKWARD, deltaTime);
  }

  if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT]) {
    camera.ProcessKeyboard(LEFT, deltaTime);
  }

  if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]) {
    camera.ProcessKeyboard(RIGHT, deltaTime);
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
