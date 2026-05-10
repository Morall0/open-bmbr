#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "model_animation.h"

class ProceduralAnimator {
public:
    // Bone names — adjust these to match your Robot.gltf bone names
    std::string rightArmBone = "Arm.R";
    std::string leftArmBone = "Arm.L";
    std::string rightHand = "Hand.R";
    std::string leftHand = "Hand.L";

    // Idle animation parameters
    float idleFrequency = 1.5f;           // Cycles per second
    float idleAmplitude = 35.0f;          // Degrees of arm swing

    // Computes bone matrices for the idle arm-swing animation
    // Call this every frame when the player is standing still
    std::vector<glm::mat4> ComputeIdle(Model& model, float currentTime);
};
#pragma once
