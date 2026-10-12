#pragma once

#include <glm/glm.hpp>

#include "scene/camera.hpp"               // `my::Camera`.

namespace my {

// A scene is a struct that holds variables. It's used when callbacks are called. 
// The glfwWindowUserPointer must be set to a Scene pointer for things to work.
struct Scene {
    Camera *camera;
    bool wireframe = false; //The style of drawin. If true, draw only the edges

    float moon_scale = 0.01f;
    float heart_angle = 0.0f;
};
}