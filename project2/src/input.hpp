#pragma once
// --- INPUT ---
// Handling every key that acts on the scene, the camera's included.
#include <glad/gl.h>                      // GLAD: must be included before GLFW.
#include <GLFW/glfw3.h>                   // GL FrameWork: `GLFWwindow`, key codes and key state.

#include "scene/camera.hpp"               // `Camera`.


// Called by GLFW once per key EVENT (press, repeat or release), during `glfwPollEvents()`.
// Suited for toggles, which must change once per press no matter how long the key is held.
// TODO: requirement 10 ('p' shows/hides the polygon mesh).
inline void keyCallback(
	[[maybe_unused]] GLFWwindow* window  ,
	[[maybe_unused]] int         key     ,  // Which key (e.g.: `GLFW_KEY_P`).
	[[maybe_unused]] int         scancode,  // Platform-specific key code (unused: `key` is portable).
	[[maybe_unused]] int         action  ,  // `GLFW_PRESS`, `GLFW_REPEAT` or `GLFW_RELEASE`.
	[[maybe_unused]] int         mods       // Modifier keys held at the time (Shift, Ctrl, ...).
) {
	
}

// Registers the callbacks above. Must be called once, after the window is created.
inline void setupInput(GLFWwindow* window) {
	glfwSetKeyCallback(window, keyCallback);
}

// Reads which keys are HELD this frame. Called once per frame, after `glfwPollEvents()`.
// Suited for continuous actions, which must progress for as long as the key is held.
// TODO: requirement 7 (scale, rotation and translation, each on a different model, each on its own keys).
// TODO: camera controls.
inline void updateInput(
	[[maybe_unused]] Camera&     camera   ,
	[[maybe_unused]] GLFWwindow* window   ,
	[[maybe_unused]] float       deltaTime   // Seconds since the last frame: keeps speeds independent of the frame rate.
) {
	
}
