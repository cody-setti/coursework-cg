#pragma once

#include <glm/ext/vector_float3.hpp>

//Just some place to store constants and not pollute the othe codes
namespace my {
    
const glm::vec3 HUT_POS = glm::vec3(0.0f, 2.1f, 0.0f);
const glm::vec3 OAK_POS = glm::vec3(5.0f, 2.78f, -3.7f);
const glm::vec3 PINE_POS = glm::vec3(-9.0f, 2.0f, -12.0f);

const glm::vec3 MASK_POS = glm::vec3(0.0f, 2.19f, 0.0f);
const glm::vec3 MASK_SCALE = glm::vec3(0.01f);

const glm::vec3 POT_SCALE = glm::vec3(0.03f);
const glm::vec3 POT1_POS = glm::vec3(1.8f, 2.19, -0.8f);
const glm::vec3 POT2_POS = glm::vec3(1.8f, 2.19, 0.6f);

const glm::vec3 HEART_SCALE = glm::vec3(0.022f);
const glm::vec3 HEART_POS = glm::vec3(1.8f, 2.5, -0.1f);
const float     HEART_ROTATION_SPEED = 2.0f;


const glm::vec3 EPONA_POS_INI = glm::vec3(-7.0f, 2.0f, -10.0f);
const float     EPONA_SPEED = 7.0f;

const glm::vec3 MOON_POS = glm::vec3(-20.0f, 70.0f, 0.0f);
const float     MOON_SCALE_SPEED = 0.005f;



const glm::vec3 CAMERA_POS_INI = glm::vec3(0.0f, 3.0f, 12.0f);
const glm::vec3 CAMERA_UP = glm::vec3(0.0f, 1.0f, 0.0f);
const glm::vec3 CAMERA_FRONT_INI = glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f));
const float     CAMERA_SPEED = 5.0f;

const glm::vec3 LOWER_BBOX = glm::vec3(-10.0f, 3.0f, -10.0f);
const glm::vec3 HIGHER_BBOX = glm::vec3(10.0f, 6.0f, 10.0f);

const float     MOUSE_SENSIVITY = 0.1f;
}