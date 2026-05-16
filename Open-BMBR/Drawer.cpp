#include "Drawer.hpp"
#include "stb_image.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

Drawer::Drawer(Shader& lightingShader, Shader& skeletalAnimShader, Shader& modelLoadingShader)
  : lightingShader(lightingShader), skeletalAnimShader(skeletalAnimShader), modelLoadingShader(modelLoadingShader) 
{
}

void Drawer::InitMapMaterials() {
  // Load map textures
  GLuint ground_diff  = LoadTexture2D("images/ground_diffuse.png");
  GLuint ground_rough = LoadTexture2D("images/ground_rough.png");
  GLuint ground_ao    = LoadTexture2D("images/ground_ao.png");

  // Wall PBR maps
  GLuint wall_diff    = LoadTexture2D("images/wall_diffuse.png");
  GLuint wall_rough   = LoadTexture2D("images/wall_rough.png");
  GLuint wall_ao      = LoadTexture2D("images/wall_diffuse.png");

  // Brick PBR maps
  GLuint brick_diff  = LoadTexture2D("images/brick_diffuse.png"); 
  GLuint brick_rough = LoadTexture2D("images/brick_rough.png");
  GLuint brick_ao    = LoadTexture2D("images/brick_ao.png");

  map_materials.ground_mat = {ground_diff, ground_rough, ground_ao, 16.0f, 29.0f, 11.0f};
  map_materials.wall_mat   = {wall_diff,   wall_rough,   wall_ao,   64.0f, 0.5f,  0.5f};
  map_materials.brick_mat  = {brick_diff,  brick_rough,  brick_ao,  16.0f, 0.5f,  0.5f};
}

void Drawer::SetupLights(Bomb& bomb, GLfloat currentFrame) {
  bool bombActive = bomb.getBombState();
  bool fireActive = bomb.isFireActive();

  float bombBlinkScale = sin(currentFrame * 6.0f) * 0.05f;
  float bombLightIntensity = 0.1f + bombBlinkScale * 10.0f;
  bombLightIntensity = glm::clamp(bombLightIntensity, 0.0f, 1.2f);

  glm::vec3 bpos = bomb.getBombPosition() + glm::vec3(0.0f, 0.5f, 0.0f);
  glm::vec3 fpos = bomb.getBombPosition() + glm::vec3(0.0f, 0.3f, 0.0f);

  // Update dynamic light positions and intensities for all shaders
  Shader* shaders[] = {&lightingShader, &skeletalAnimShader, &modelLoadingShader};

  for (Shader* shader : shaders) {
    shader->use();
    shader->setBool("bombLightActive", bombActive);
    if (bombActive) {
      shader->setVec3("bombLight.position", bpos);
      shader->setFloat("bombLight.constant",  1.0f);
      shader->setFloat("bombLight.linear",    0.35f);
      shader->setFloat("bombLight.quadratic", 0.44f);
      shader->setVec3("bombLight.ambient",  bombLightIntensity * 0.3f, bombLightIntensity * 0.15f, 0.0f);
      shader->setVec3("bombLight.diffuse",  bombLightIntensity * 1.0f, bombLightIntensity * 0.5f,  0.0f);
      shader->setVec3("bombLight.specular", bombLightIntensity * 0.5f, bombLightIntensity * 0.25f, 0.0f);
    }

    shader->setBool("fireLightActive", fireActive);
    if (fireActive) {
      shader->setVec3("fireLight.position",  fpos);
      shader->setFloat("fireLight.constant",  1.0f);
      shader->setFloat("fireLight.linear",    (shader == &lightingShader) ? 0.09f : 0.14f);
      shader->setFloat("fireLight.quadratic", (shader == &lightingShader) ? 0.032f : 0.07f);
      shader->setVec3("fireLight.ambient",    (shader == &lightingShader) ? glm::vec3(0.8f, 0.3f, 0.0f) : glm::vec3(0.6f, 0.25f, 0.0f));
      shader->setVec3("fireLight.diffuse",    (shader == &lightingShader) ? glm::vec3(2.5f, 1.0f, 0.0f) : glm::vec3(2.0f, 0.8f, 0.0f));
      shader->setVec3("fireLight.specular",   1.0f, 0.5f, 0.0f);
    }
  }
}

void Drawer::DrawMap(const Map& map) {
  lightingShader.use();
  DrawFloor();
  DrawWalls();
  DrawMapBlocks(map);
}

void Drawer::DrawFloor() {
  glm::mat4 model(1.0f);
  ApplyTexture(map_materials.ground_mat, lightingShader);
  model = glm::scale(model, glm::vec3(MAP_COLS, 1.0f, MAP_ROWS));
  model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
  lightingShader.setMat4("model", model);
  glDrawArrays(GL_TRIANGLES, 30, 6);
}

void Drawer::DrawWalls() {
  glm::mat4 model(1.0f);
  ApplyTexture(map_materials.wall_mat, lightingShader);

  for (float x = -HALF_COLS; x <= HALF_COLS; x += 1.0f) {
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, 0.0f, HALF_ROWS));
    lightingShader.setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, 0.0f, -HALF_ROWS));
    lightingShader.setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
  }

  for (float z = -HALF_ROWS + 1.0f; z <= HALF_ROWS - 1.0f; z += 1.0f) {
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(HALF_COLS, 0.0f, z));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    lightingShader.setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-HALF_COLS, 0.0f, z));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    lightingShader.setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
  }
}

void Drawer::DrawMapBlocks(const Map& map) {
  glm::mat4 model(1.0f);

  for (int row = 0; row < MAP_ROWS; row++) {
    for (int col = 0; col < MAP_COLS; col++) {
      int cell = map.getCell(row, col);
      if (cell == 0 || cell == 5 || cell == 6 || cell == 7 || cell == 8)
        continue;

      float xPos = col - (HALF_COLS - 1);
      float zPos = row - (HALF_ROWS - 1);

      if (cell == 1) {
        ApplyTexture(map_materials.wall_mat, lightingShader);
      } else if (cell >= 2 && cell <= 4) {
        ApplyTexture(map_materials.brick_mat, lightingShader);
      }

      model = glm::mat4(1.0f);
      model = glm::translate(model, glm::vec3(xPos, 0.0f, zPos));
      lightingShader.setMat4("model", model);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }
  }
}

void Drawer::DrawBomb(Bomb& bomb, Map& map, GLfloat currentTime, Model& bombModel) {
  if (bomb.getBombState()) {
    float blinkSpeed = 6.0f;
    float blinkIntensity = 0.05f;
    float blinkScale = sin(currentTime * blinkSpeed) * blinkIntensity;
    float finalBlinkScale = 0.15f + (blinkScale * 0.5f);

    glm::vec3 bomb_position = bomb.getBombPosition();
    if (currentTime >= bomb.getBombExpiration()) {
      bomb.expireBomb(bomb_position, map, currentTime);
    }

    modelLoadingShader.use();
    glm::mat4 model(1.0f);
    model = glm::translate(model, bomb_position);
    model = glm::scale(model, glm::vec3(finalBlinkScale, finalBlinkScale, finalBlinkScale));
    modelLoadingShader.setMat4("model", model);
    bombModel.Draw(modelLoadingShader);
  }
}

void Drawer::DrawFire(Bomb& bomb, Map& map, GLfloat currentTime, Model& fireModel, Animator& fireAnimator) {
  if (bomb.isFireActive()) {
    glm::vec3 firePosition = bomb.getBombPosition() - glm::vec3(0.0f, 0.25f, 0.0f);
    MapIndices grid_position = map.toMapIndices(firePosition);

    if (currentTime >= bomb.getFireExpiration()) {
      bomb.putOutFire(grid_position, map);
    }

    skeletalAnimShader.use();
    auto transforms = fireAnimator.GetFinalBoneMatrices();
    for (size_t i = 0; i < transforms.size(); ++i) {
      skeletalAnimShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
    }

    glm::vec3 center = firePosition + glm::vec3(0.0f, 0.45f, 0.0f);
    skeletalAnimShader.setInt("isFireModel", 1);

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
      // Verify if X axis flames are free
      bool posXfree = map.getCell(map.toMapIndices(posX).row, map.toMapIndices(posX).col) == 6;
      bool negXfree = map.getCell(map.toMapIndices(negX).row, map.toMapIndices(negX).col) == 6;

      if (posXfree) {
        model = glm::mat4(1.0f);
        model = glm::translate(model, firePosition + glm::vec3(0.5f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.04f * 3.0f, 0.04f, 0.04f));
        skeletalAnimShader.setMat4("model", model);
        skeletalAnimShader.setVec3("fireCenter", center);
        fireModel.Draw(skeletalAnimShader);
      }
      if (negXfree) {
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
      // Verify if Z axis flames are free
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

    skeletalAnimShader.setInt("isFireModel", 0);
  }
}

void Drawer::DrawDoor(const Map& map, Model& hatchModel) {
  modelLoadingShader.use();
  modelLoadingShader.setFloat("shininess", 32.0f);

  for (int row = 0; row < map.getTotalRows(); row++) {
    for (int col = 0; col < map.getTotalCols(); col++) {
      if (map.getCell(row, col) != 7)
        continue;

      float xPos = col - (HALF_COLS - 1);
      float zPos = row - (HALF_ROWS - 1);

      glm::vec3 doorWorldPos(xPos, -0.48f, zPos);

      glm::mat4 model(1.0f);
      model = glm::translate(model, doorWorldPos);
      model = glm::scale(model, glm::vec3(0.35f, 0.35f, 0.35f));
      modelLoadingShader.setMat4("model", model);

      hatchModel.Draw(modelLoadingShader);
    }
  }
}

void Drawer::DrawRobot(Player& bomberman, Model& robot, Animator& animator, bool isFirstPerson) {
  skeletalAnimShader.use();

  auto transforms = animator.GetFinalBoneMatrices();
  for (size_t i = 0; i < transforms.size(); ++i) {
    skeletalAnimShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
  }

  if (!isFirstPerson) {
    glm::mat4 model(1.0f);
    model = glm::translate(model, bomberman.getPosition()) *
      glm::toMat4(bomberman.getOrientation());
    model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
    skeletalAnimShader.setMat4("model", model);
    skeletalAnimShader.setInt("isFireModel", 0);
    skeletalAnimShader.setFloat("shininess", 96.0f); // metal robot
    robot.Draw(skeletalAnimShader);
  }
}

void Drawer::DrawEnemies(const std::vector<Enemy>& enemies, GLfloat currentFrame, Model& ballomModel, Model& onilCuerpoModel, Model& onilPieIzqModel, Model& onilPieDerModel) {
  modelLoadingShader.use();

  for (const auto &enemy : enemies) {
    glm::mat4 model(1.0f);

    float finalScale = 0.4f;
    float yOffset = 0.0f;

    if (enemy.getType() == EnemyType::BALLOM) {
      float inflationSpeed = 2.0f;
      float inflationIntensity = 0.05f;
      finalScale = enemy.getIsDead() ? enemy.getDeathScale() : (0.4f + sin(currentFrame * inflationSpeed) * inflationIntensity);

      float bobbingSpeed = 2.0f; 
      float bobbingIntensity = 0.15f;
      float bobbing = enemy.getIsDead() ? 0.0f : abs(sin(currentFrame * bobbingSpeed)) * bobbingIntensity;

      yOffset = 0.55f + bobbing;
    } else {
      finalScale = enemy.getIsDead() ? enemy.getDeathScale() : 0.4f;
      yOffset = 0.3f;
    }

    glm::vec3 drawPosition = enemy.getPosition() + glm::vec3(0.0f, yOffset, 0.0f);

    model = glm::translate(model, drawPosition) * glm::toMat4(enemy.getOrientation()); 
    model = glm::scale(model, glm::vec3(finalScale, finalScale, finalScale));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    modelLoadingShader.setMat4("model", model);
    if (enemy.getType() == EnemyType::BALLOM) {
      modelLoadingShader.setFloat("shininess", 64.0f);
      ballomModel.Draw(modelLoadingShader);
    } else {
      modelLoadingShader.setFloat("shininess", 8.0f);
      onilCuerpoModel.Draw(modelLoadingShader);

      float swingAngle = 0.0f;
      if (enemy.getIsMoving() && !enemy.getIsDead()) {
        float walkSpeed = 6.0f;
        float walkIntensity = 0.5f;
        swingAngle = sin(currentFrame * walkSpeed) * walkIntensity;
      }

      glm::mat4 leftFootModel = model;
      leftFootModel = glm::rotate(leftFootModel, swingAngle, glm::vec3(0.0f, 0.0f, 1.0f));
      modelLoadingShader.setMat4("model", leftFootModel);
      onilPieIzqModel.Draw(modelLoadingShader);

      glm::mat4 rightFootModel = model;
      rightFootModel = glm::rotate(rightFootModel, -swingAngle, glm::vec3(0.0f, 0.0f, 1.0f));
      modelLoadingShader.setMat4("model", rightFootModel);
      onilPieDerModel.Draw(modelLoadingShader);
    }
  }
}

void Drawer::ApplyTexture(const Material& material, Shader& shader) {
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, material.diffuse);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, material.specular);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, material.ao);

  shader.setFloat("material.shininess", material.shininess);
  shader.setVec2("uvScale", material.uvScaleX, material.uvScaleY);
}

GLuint Drawer::LoadTexture2D(const char* path) {
  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  int textureWidth, textureHeight, nrChannels;
  unsigned char* image = stbi_load(path, &textureWidth, &textureHeight, &nrChannels, 0);

  if (!image) {
    std::cout << "Failed to load texture: " << path << std::endl;
    glDeleteTextures(1, &texture);
    return 0;
  }

  GLenum internalFormat, dataFormat;
  if (nrChannels == 1) {
    internalFormat = GL_R8;
    dataFormat     = GL_RED;
  } else if (nrChannels == 2) {
    stbi_image_free(image);
    image = stbi_load(path, &textureWidth, &textureHeight, &nrChannels, 1);
    internalFormat = GL_R8;
    dataFormat     = GL_RED;
  } else if (nrChannels == 3) {
    internalFormat = GL_RGB8;
    dataFormat     = GL_RGB;
  } else if (nrChannels == 4) {
    internalFormat = GL_RGBA8;
    dataFormat     = GL_RGBA;
  } else {
    std::cout << "Unsupported channel count (" << nrChannels << ") for: " << path << std::endl;
    stbi_image_free(image);
    glDeleteTextures(1, &texture);
    return 0;
  }

  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, textureWidth, textureHeight,
      0, dataFormat, GL_UNSIGNED_BYTE, image);
  glGenerateMipmap(GL_TEXTURE_2D);

  stbi_image_free(image);
  return texture;
}

GLuint Drawer::CreateSolidTexture(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
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
