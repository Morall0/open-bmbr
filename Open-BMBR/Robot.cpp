#include "Robot.h"

#include <cmath>
#define GLM_ENABLE_EXPERIMENTAL 
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/constants.hpp>

std::vector<glm::mat4> ProceduralAnimator::ComputeIdle(Model& model, float currentTime) {
    auto& boneInfoMap = model.GetBoneInfoMap();
    int   boneCount = model.GetBoneCount();

    // Start every bone with identity (no extra transformation)
    std::vector<glm::mat4> finalMatrices(boneCount, glm::mat4(1.0f));

    // Oscillation value in [-1, 1]
    // Right arm goes forward (+swing), left arm goes backward (-swing)
    float swing = std::sin(currentTime * idleFrequency * glm::two_pi<float>());
    float swingAngle = glm::radians(idleAmplitude) * swing;

    //Bajar los brazos
    float baseDownAngle = glm::radians(-160.0f); //glm::radians(-80.0f);

    for (auto& [name, boneInfo] : boneInfoMap) {
        //glm::mat4 localRotation(1.0f);
        //glm::quat q;

        if (name == rightArmBone || name == rightHand) {
            // Rotate around X axis: positive = arm swings forward
            //glm::quat q = glm::angleAxis(angleRad, glm::vec3(1.0f, 0.0f, 0.0f));
            //localRotation = glm::toMat4(q);
            //q = glm::angleAxis(angleRad, glm::vec3(0.0f, 0.0f, 1.0f));
            //finalMatrices[boneInfo.id] = glm::toMat4(q);
            glm::quat baseDown = glm::angleAxis(baseDownAngle, glm::vec3(0.0f, 1.0f, 0.0f));
            glm::quat swing_q = glm::angleAxis(swingAngle, glm::vec3(1.0f, 0.0f, 0.0f));
            finalMatrices[boneInfo.id] = glm::toMat4(baseDown * swing_q);
        }
        else if (name == leftArmBone  || name == leftHand) {
            // Opposite phase so arms alternate
            //glm::quat q = glm::angleAxis(-angleRad, glm::vec3(1.0f, 0.0f, 0.0f));
            //localRotation = glm::toMat4(q);
            //q = glm::angleAxis(-angleRad, glm::vec3(0.0f, 0.0f, 1.0f));
            //finalMatrices[boneInfo.id] = glm::toMat4(q);
            glm::quat baseDown = glm::angleAxis(baseDownAngle, glm::vec3(0.0f, 1.0f, 0.0f));
            glm::quat swing_q = glm::angleAxis(-swingAngle, glm::vec3(1.0f, 0.0f, 0.0f));
            finalMatrices[boneInfo.id] = glm::toMat4(baseDown * swing_q);
        }

        // offset brings the bone from model-space to bone-space,
        // then we apply our local rotation on top
        //finalMatrices[boneInfo.id] = boneInfo.offset * localRotation;
    }

    return finalMatrices;
}