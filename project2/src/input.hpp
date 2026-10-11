#pragma once
// --- INPUT ---
// Handling every key that acts on the scene, the camera's included.
#include <algorithm>					  // using std::clamp
#include <glad/gl.h>                      // GLAD: must be included before GLFW.
#include <GLFW/glfw3.h>                   // GL FrameWork: `GLFWwindow`, key codes and key state.
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/glm.hpp>

#include "scene/camera.hpp"               // `my::Camera`.
#include "scene/constants.hpp" 			  // Constants regarding the objects and their movimentations
#include "scene/scene.hpp"               // `


namespace my {

// Called by GLFW once per key EVENT (press, repeat or release), during `glfwPollEvents()`.
// Suited for toggles, which must change once per press no matter how long the key is held.
inline void keyCallback(
	[[maybe_unused]] GLFWwindow* window  ,
	[[maybe_unused]] int         key     ,  // Which key (e.g.: `GLFW_KEY_P`).
	[[maybe_unused]] int         scancode,  // Platform-specific key code (unused: `key` is portable).
	[[maybe_unused]] int         action  ,  // `GLFW_PRESS`, `GLFW_REPEAT` or `GLFW_RELEASE`.
	[[maybe_unused]] int         mods       // Modifier keys held at the time (Shift, Ctrl, ...).
) {
	Scene *scene = static_cast<Scene*>(glfwGetWindowUserPointer(window));

	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if(key == GLFW_KEY_P && action == GLFW_PRESS)
		scene->wireframe = !scene->wireframe;
}

//callBack of the mouse position
inline void mouseCallback(
	[[maybe_unused]] GLFWwindow* window, 
	[[maybe_unused]] double xpos, //position x of the mouse in the window
	[[maybe_unused]]double ypos   //position y of the mouse in the window
) {
	Scene *scene = static_cast<Scene*>(glfwGetWindowUserPointer(window));
	Camera *camera = scene->camera;

	static double last_pos_x = xpos; //static so it holds its value inbetween calls. This is only called at the first time this function is called
    static double last_pos_y = ypos;

    double xoffset = xpos - last_pos_x;
    double yoffset = last_pos_y - ypos;

    last_pos_x = xpos;
    last_pos_y = ypos;

	xoffset *= MOUSE_SENSIVITY;
	yoffset *= MOUSE_SENSIVITY;
	camera->yaw += xoffset;
    camera->pitch += yoffset;
	
	camera->pitch = std::clamp(camera->pitch, -89.0, 89.0);
	calculateFront(*camera);
}
//TODO: change FOV


// Registers the callbacks above. Must be called once, after the window is created.
inline void setupInput(GLFWwindow* window) {
	glfwSetKeyCallback(window, my::keyCallback);
	glfwSetCursorPosCallback(window, my::mouseCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}


bool held(GLFWwindow* window, int key) {
	return glfwGetKey(window, key) == GLFW_PRESS;
}
// Reads which keys are HELD this frame. Called once per frame, after `glfwPollEvents()`.
// Suited for continuous actions, which must progress for as long as the key is held.
// TODO: requirement 7 (scale, rotation and translation, each on a different model, each on its own keys).
inline void updateInput(
	[[maybe_unused]] my::Camera& camera   ,
	[[maybe_unused]] GLFWwindow* window   ,
	[[maybe_unused]] float       deltaTime   // Seconds since the last frame: keeps speeds independent of the frame rate.
) {
	//CAMERA MOVEMENT
	glm::vec3 direction = glm::vec3(0.0f);
	if(held(window, GLFW_KEY_W)) direction = direction + camera.front;
	if(held(window, GLFW_KEY_S)) direction = direction - camera.front;
	if(held(window, GLFW_KEY_D)) direction = direction + glm::normalize(glm::cross(camera.front, camera.up));
	if(held(window, GLFW_KEY_A)) direction = direction - glm::normalize(glm::cross(camera.front, camera.up));
	direction.y = 0.0f;
	if(glm::length(direction) > 0.0f) {
		direction = glm::normalize(direction);
		camera.position = camera.position + (CAMERA_SPEED * direction * deltaTime);
	}
	camera.position.x = std::clamp(camera.position.x, LOWER_BBOX.x, HIGHER_BBOX.x);
	camera.position.z = std::clamp(camera.position.z, LOWER_BBOX.z, HIGHER_BBOX.z);
}


}  // namespace my
