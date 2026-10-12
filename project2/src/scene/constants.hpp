#pragma once

#include <glm/ext/vector_float3.hpp>
#include <vector>

//Just some place to store constants and not pollute the othe codes
namespace my {


//GRASS
const glm::vec3 GRASS_POS = glm::vec3(0.0f, 0.0f, 0.0f);
const glm::vec3 GRASS_SCALE = glm::vec3(100.0f);
    

//HUT
const glm::vec3 HUT_POS = glm::vec3(0.0f, 0.0f, 0.0f);


//TREES
const std::vector<glm::vec3> OAK_POS = {glm::vec3(6.0f, 0.0f, 0.0f),
                                        glm::vec3(0.0f, 0.0f, -5.0f),
                                        glm::vec3(-5.0f, 0.0f, 6.0f),};

const std::vector<glm::vec3> OAK_SCALE = {glm::vec3(1.2f),
                                        glm::vec3(0.9f),
                                        glm::vec3(1.6f)};

const std::vector<glm::vec3> PINE_POS = {glm::vec3(-9.0f, 0.0f, -12.0f),
                                         glm::vec3(-5.0f, 0.0f, -9.0f),
                                         glm::vec3(5.0f, 0.0f, -7.0f),
                                         glm::vec3(5.0f, 0.0f, 7.0f),
                                         glm::vec3(0.0f, 0.0f, 14.0f),
                                         glm::vec3(10.0f, 0.0f, 9.0f),
                                         glm::vec3(-7.0f, 0.0f, 13.0f),
                                         glm::vec3(-10.0f, 0.0f, 0.0f),
                                         glm::vec3(-11.0f, 0.0f, 4.0f),};

const std::vector<glm::vec3> PINE_SCALE = {glm::vec3(1.6f),
                                         glm::vec3(1.5f),
                                         glm::vec3(3.0f),
                                         glm::vec3(1.1f),
                                         glm::vec3(1.2f),
                                         glm::vec3(1.5f),
                                         glm::vec3(1.7f),
                                         glm::vec3(2.0f),
                                         glm::vec3(1.0f),};


//MASK SALESMAN
const glm::vec3 MASK_POS = glm::vec3(0.0f, 0.03f, 0.0f);
const glm::vec3 MASK_SCALE = glm::vec3(0.01f);


//POTS
const glm::vec3 POT_SCALE = glm::vec3(0.03f);
const glm::vec3 POT1_POS = glm::vec3(1.8f, 0.03, -0.8f);
const glm::vec3 POT2_POS = glm::vec3(1.8f, 0.03, 0.6f);


//SHIELD
const glm::vec3 SHIELD_SCALE = glm::vec3(0.2f);
const glm::vec3 SHIELD_POS = glm::vec3(-2.5f, 0.0f, 0.6f);
const glm::vec3 SHIELD_ANGLES = glm::vec3(0.0f, glm::radians(-90.0f), glm::radians(-17.0f));


//PIECE OF HEART
const glm::vec3 HEART_SCALE = glm::vec3(0.022f);
const glm::vec3 HEART_POS = glm::vec3(1.8f, 0.3f, -0.1f);
const float     HEART_ROTATION_SPEED = 4.0f;


//EPONA
const glm::vec3 EPONA_SCALE = 0.0015f * glm::vec3(-1.0f, 1.0f, -1.0f);
const float     EPONA_SPEED = 7.0f;


//MOON
const glm::vec3 MOON_POS = glm::vec3(-20.0f, 70.0f, 0.0f);
const float     MOON_SCALE_SPEED = 0.005f;
const float     MAX_MOON_SCALE = 0.03f;
const float     MIN_MOON_SCALE = 0.005f;


//CAMERA
const glm::vec3 CAMERA_POS_INI = glm::vec3(0.0f, 0.9f, 12.0f);
const glm::vec3 CAMERA_UP = glm::vec3(0.0f, 1.0f, 0.0f);
const glm::vec3 CAMERA_FRONT_INI = glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f));
const float     CAMERA_SPEED = 5.0f;
const float     CAMERA_SPEED_SPRINT = 10.0f;

const float     MAX_FOV = 90.0f;
const float     MIN_FOV = 22.5f;

const glm::vec3 LOWER_BBOX = glm::vec3(-10.0f, 0.8f, -10.0f);
const glm::vec3 HIGHER_BBOX = glm::vec3(10.0f, 3.0f, 10.0f);

const float     MOUSE_SENSIVITY = 0.1f;
}