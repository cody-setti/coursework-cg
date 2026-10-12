#pragma once

#include <glm/glm.hpp>

#include "scene/camera.hpp"               // `my::Camera`.

namespace my {

// A scene is a struct that holds variables. It's used when callbacks are called. 
// The glfwWindowUserPointer must be set to a Scene pointer for things to work.
struct Scene {
    Camera *camera;
    float fov = 60.0f;
    int window_width = 1280;
    int window_height = 720;
    
    bool wireframe = false; //The style of drawin. If true, draw only the edges

    float moon_scale = 0.01f;
    float heart_angle = 0.0f;

    glm::vec3 epona_pos = glm::vec3(-3.0f, 0.0f, 7.0f);
    glm::vec3 epona_last_direction = glm::vec3(0.0f, 0.0f, -1.0f);
};
}