#include "Drawer.hpp"
#include "Map.hpp"
#include "stb_image.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

Drawer::Drawer(Shader& lightingShader, Shader& skeletalAnimShader, Shader& modelLoadingShader)
  : lightingShader(lightingShader), skeletalAnimShader(skeletalAnimShader), modelLoadingShader(modelLoadingShader)
{
}

void Drawer::InitMapMaterials() {
  // Load map textures (single diffuse per material)
  GLuint ground_diff    = LoadTexture2D("images/grass.png");
  GLuint wall_diff      = LoadTexture2D("images/wall.png");
  GLuint hardblock_diff = LoadTexture2D("images/hardblock.png");
  GLuint brick_diff     = LoadTexture2D("images/brick.png");

  // Alternative door texture
  doorAlternativeTex = LoadTexture2D("images/open_portal_shield.png");

  //                              diffuse        Ka     Ks    shininess  uvX    uvY
  map_materials.ground_mat    = {ground_diff,    0.7f,  0.10f, 16.0f,  14.5f, 5.5f};
  map_materials.wall_mat      = {wall_diff,      0.6f,  0.20f, 32.0f,   1.0f, 1.0f};
  map_materials.hardblock_mat = {hardblock_diff, 0.6f,  0.80f, 128.0f,  1.0f, 1.0f};
  map_materials.brick_mat     = {brick_diff,     0.65f, 0.10f, 16.0f,   1.0f, 1.0f};

  // Powerup Textures
  GLuint bomb_pu_tex  = LoadTexture2D("images/pwup_bomb.png");
  GLuint fire_pu_tex  = LoadTexture2D("images/pwup_fire.png");
  GLuint speed_pu_tex = LoadTexture2D("images/pwup_speed.png");

  map_materials.bomb_pu_mat   = {bomb_pu_tex,  0.8f, 0.5f, 64.0f, 1.0f, 1.0f};
  map_materials.fire_pu_mat   = {fire_pu_tex,  0.8f, 0.5f, 64.0f, 1.0f, 1.0f};
  map_materials.speed_pu_mat  = {speed_pu_tex, 0.8f, 0.5f, 64.0f, 1.0f, 1.0f};
}

void Drawer::SetupLights(std::vector<Bomb>& bombs, GLfloat currentFrame) {
  bool bombActive = false;
  bool fireActive = false;
  glm::vec3 bpos(0.0f);
  glm::vec3 fpos(0.0f);

  for (Bomb& bomb : bombs) {
    if (bomb.getBombState()) {
      bombActive = true;
      bpos = bomb.getBombPosition() + glm::vec3(0.0f, 0.5f, 0.0f);
    }
    if (bomb.isFireActive()) {
      fireActive = true;
      fpos = bomb.getBombPosition() + glm::vec3(0.0f, 0.3f, 0.0f);
    }
  }

  // Update dynamic light positions and intensities for all shaders
  Shader* shaders[] = {&lightingShader, &skeletalAnimShader, &modelLoadingShader};

  for (Shader* shader : shaders) {
    shader->use();
    shader->setBool("bombLightActive", bombActive);
    if (bombActive) {
      float bombBlinkScale = sin(currentFrame * 6.0f) * 0.05f;
      float bombLightIntensity = glm::clamp(0.1f + bombBlinkScale * 10.0f, 0.0f, 1.2f);

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

void Drawer::DrawMap(const Map& map, GLfloat currentFrame) {
  lightingShader.use();
  DrawFloor();
  DrawWalls();
  DrawMapBlocks(map, currentFrame);
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

void Drawer::DrawMapBlocks(const Map& map, GLfloat currentFrame) {
  glm::mat4 model(1.0f);

  for (int row = 0; row < MAP_ROWS; row++) {
    for (int col = 0; col < MAP_COLS; col++) {
      int cell = map.getCell(row, col);
      if (cell == 0 || cell == 5 || cell == 6 || cell == 7)
        continue;

      float xPos = col - (HALF_COLS - 1);
      float zPos = row - (HALF_ROWS - 1);

      if (cell == 1) {
        ApplyTexture(map_materials.hardblock_mat, lightingShader);
      } else if (cell == 2 || cell == 3 || cell == 4 || cell == 9 || cell == 10) {
        ApplyTexture(map_materials.brick_mat, lightingShader);
      } else if (cell == 8) {
        ApplyTexture(map_materials.bomb_pu_mat, lightingShader);
      } else if (cell == 11) {
        ApplyTexture(map_materials.fire_pu_mat, lightingShader);
      } else if (cell == 12) {
        ApplyTexture(map_materials.speed_pu_mat, lightingShader);
      } else {
        continue;
      }

      model = glm::mat4(1.0f);

      // Floating & rotating animation
      if (cell == 8 || cell == 11 || cell == 12) {
        float bobHeight = 0.15f;
        float bobSpeed = 3.0f;
        float rotSpeed = 2.0f;
        float yPos = -0.2f + sin(currentFrame * bobSpeed) * bobHeight;

        model = glm::translate(model, glm::vec3(xPos, yPos, zPos));
        model = glm::rotate(model, currentFrame * rotSpeed, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
      } else {
        model = glm::translate(model, glm::vec3(xPos, 0.0f, zPos));
      }

      lightingShader.setMat4("model", model);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }
  }
}

void Drawer::DrawBomb(std::vector<Bomb>& bombs, Map& map, GLfloat currentTime, Model& bombModel) {
  for (Bomb& bomb : bombs) {
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
      modelLoadingShader.setFloat("shininess", 128.0f); // bowling-ball polish
      bombModel.Draw(modelLoadingShader);
    }
  }
}

void Drawer::DrawFire(std::vector<Bomb>& bombs, Map& map, GLfloat currentTime, Model& fireModel, Animator& fireAnimator) {
  for (Bomb& bomb : bombs) {
    if (bomb.isFireActive()) {
      glm::vec3 firePosition = bomb.getBombPosition() - glm::vec3(0.0f, 0.25f, 0.0f);
      MapIndices grid_position = map.toMapIndices(firePosition);

      if (currentTime >= bomb.getFireExpiration()) {
        bomb.putOutFire(grid_position, map);
        continue;
      }

      skeletalAnimShader.use();
      std::vector<glm::mat4> transforms = fireAnimator.GetFinalBoneMatrices();
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

      // Cross flames X and Z axis
      int blastRadius = bomb.getBlastRadius();

      for (int dir = -1; dir <= 1; dir += 2) {
        // X axis
        for (int step = 1; step <= blastRadius; step++) {
          glm::vec3 pos = firePosition + glm::vec3(dir * step, 0.0f, 0.0f);
          if (map.getCell(map.toMapIndices(pos).row, map.toMapIndices(pos).col) == 6) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, firePosition + glm::vec3(dir * step * 0.5f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.04f * 3.0f * step, 0.04f, 0.04f));
            skeletalAnimShader.setMat4("model", model);
            skeletalAnimShader.setVec3("fireCenter", center);
            fireModel.Draw(skeletalAnimShader);
          } else {
            break;
          }
        }
        // Z axis
        for (int step = 1; step <= blastRadius; step++) {
          glm::vec3 pos = firePosition + glm::vec3(0.0f, 0.0f, dir * step);
          if (map.getCell(map.toMapIndices(pos).row, map.toMapIndices(pos).col) == 6) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, firePosition + glm::vec3(0.0f, 0.0f, dir * step * 0.5f));
            model = glm::scale(model, glm::vec3(0.04f, 0.04f, 0.04f * 3.0f * step));
            skeletalAnimShader.setMat4("model", model);
            skeletalAnimShader.setVec3("fireCenter", center);
            fireModel.Draw(skeletalAnimShader);
          } else {
            break;
          }
        }
      }

      skeletalAnimShader.setInt("isFireModel", 0);
    }
  }
}

void Drawer::DrawDoor(const Map& map, Model& doorBaseModel, Model& doorModel, Animator& animator, bool locked) {
  MapIndices door_index = map.getDoorPosition();

  float xPos = door_index.col - (HALF_COLS - 1);
  float zPos = door_index.row - (HALF_ROWS - 1);

  glm::vec3 doorWorldPos(xPos, -2.5f, zPos);

  // Draw door base
  modelLoadingShader.use();
  glm::mat4 model(1.0f);
  model = glm::translate(model, doorWorldPos + glm::vec3(0.0f, 2.0f, 0.0f)); // Adjust the y pos to the floor
  model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));
  modelLoadingShader.setMat4("model", model);
  doorBaseModel.Draw(modelLoadingShader);

  // Draw door base
  skeletalAnimShader.use();
  skeletalAnimShader.setFloat("shininess", 96.0f); // holographic energy surface

  std::vector<glm::mat4> transforms = animator.GetFinalBoneMatrices();
  for (size_t i = 0; i < transforms.size(); ++i) {
    skeletalAnimShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
  }

  model = glm::mat4(1.0f);
  model = glm::translate(model, doorWorldPos);
  model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));
  skeletalAnimShader.setMat4("model", model);
  skeletalAnimShader.setInt("isFireModel", 0);

  // Enable alpha blending
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthMask(GL_FALSE);

  if (locked)
    doorModel.Draw(skeletalAnimShader);
  else
    doorModel.Draw(skeletalAnimShader, doorAlternativeTex);

  // Restore state
  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);
}

void Drawer::DrawRobot(Player& bomberman, Model& robot, Animator& animator, bool isFirstPerson) {
  skeletalAnimShader.use();

  std::vector<glm::mat4> transforms = animator.GetFinalBoneMatrices();
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

void Drawer::DrawEnemies(const std::vector<Enemy>& enemies, GLfloat currentFrame, Model& ballomModel, Model& onilBodyModel, Model& onilLeftFtModel, Model& onilRightFtModel) {
  modelLoadingShader.use();

  for (const Enemy& enemy : enemies) {
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
      modelLoadingShader.setFloat("shininess", 128.0f); // latex balloon
      ballomModel.Draw(modelLoadingShader);
    } else {
      modelLoadingShader.setFloat("shininess", 2.0f);
      onilBodyModel.Draw(modelLoadingShader);

      float swingAngle = 0.0f;
      if (enemy.getIsMoving() && !enemy.getIsDead()) {
        float walkSpeed = 6.0f;
        float walkIntensity = 0.5f;
        swingAngle = sin(currentFrame * walkSpeed) * walkIntensity;
      }

      glm::mat4 leftFootModel = model;
      leftFootModel = glm::rotate(leftFootModel, swingAngle, glm::vec3(0.0f, 0.0f, 1.0f));
      modelLoadingShader.setMat4("model", leftFootModel);
      onilLeftFtModel.Draw(modelLoadingShader);

      glm::mat4 rightFootModel = model;
      rightFootModel = glm::rotate(rightFootModel, -swingAngle, glm::vec3(0.0f, 0.0f, 1.0f));
      modelLoadingShader.setMat4("model", rightFootModel);
      onilRightFtModel.Draw(modelLoadingShader);
    }
  }
}

void Drawer::ApplyTexture(const Material& material, Shader& shader) {
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, material.diffuse);

  shader.setFloat("material.ambientStrength", material.ambientStrength);
  shader.setFloat("material.specularStrength", material.specularStrength);
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
