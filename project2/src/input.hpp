#pragma once
// --- INPUT ---
// Handling every key that acts on the scene, the camera's included.
#include <GL/gl.h>
#include <algorithm>					  // using std::clamp
#include <glad/gl.h>                      // GLAD: must be included before GLFW.
#include <GLFW/glfw3.h>                   // GL FrameWork: `GLFWwindow`, key codes and key state.
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/glm.hpp>

#include "scene/camera.hpp"               // `my::Camera`.
#include "scene/constants.hpp" 			  // Constants regarding the objects and their movimentations
#include "scene/scene.hpp"               // `


namespace my {

bool held(GLFWwindow* window, int key) {
	return glfwGetKey(window, key) == GLFW_PRESS;
}

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

//Capture mouse scroll movement. Used to chance fov
inline void scrollCallback(
	[[maybe_unused]] GLFWwindow* window, 
	[[maybe_unused]] double xoffset, //offset of scroll in x(unused)
	[[maybe_unused]]double yoffset   //offset of scroll in y(the usual scroll motion)
) {
	Scene *scene = static_cast<Scene*>(glfwGetWindowUserPointer(window));
	scene->fov = scene->fov - yoffset;
	scene->fov = std::clamp(scene->fov, MIN_FOV, MAX_FOV);
}

//Captures if the size of window has changed
inline void frameBufferSizeCallback(
	[[maybe_unused]] GLFWwindow* window, 
	[[maybe_unused]] int width, //window width
	[[maybe_unused]] int height  //window height
) {
	Scene *scene = static_cast<Scene*>(glfwGetWindowUserPointer(window));
	glViewport(0, 0, width, height); //makes all the pixels in the window be mapped
	//changes the aspect ratio of the projetion matrix.
	scene->window_height = height;
	scene->window_width = width;
}


// Registers the callbacks above. Must be called once, after the window is created.
inline void setupInput(GLFWwindow* window) {
	glfwSetKeyCallback(window, my::keyCallback);

	glfwSetCursorPosCallback(window, my::mouseCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwSetScrollCallback(window, scrollCallback);

	glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);
}





// Reads which keys are HELD this frame. Called once per frame, after `glfwPollEvents()`.
// Suited for continuous actions, which must progress for as long as the key is held.
inline void updateInput(
	[[maybe_unused]] my::Camera& camera   ,
	[[maybe_unused]] GLFWwindow* window   ,
	[[maybe_unused]] float       deltaTime   // Seconds since the last frame: keeps speeds independent of the frame rate.
) {
	Scene *scene = static_cast<Scene*>(glfwGetWindowUserPointer(window));

	//CAMERA MOVEMENT
	glm::vec3 direction = glm::vec3(0.0f);
	if(held(window, GLFW_KEY_W)) direction = direction + camera.front;
	if(held(window, GLFW_KEY_S)) direction = direction - camera.front;
	if(held(window, GLFW_KEY_D)) direction = direction + glm::normalize(glm::cross(camera.front, camera.up));
	if(held(window, GLFW_KEY_A)) direction = direction - glm::normalize(glm::cross(camera.front, camera.up));
	direction.y = 0.0f;
	if(held(window, GLFW_KEY_SPACE)) direction = direction + glm::vec3(0.0f, 1.0f, 0.0f);
	if(held(window, GLFW_KEY_LEFT_CONTROL)) direction = direction - glm::vec3(0.0f, 1.0f, 0.0f);
	if(glm::length(direction) > 0.0f) {
		direction = glm::normalize(direction);
		if(held(window, GLFW_KEY_LEFT_SHIFT))
			camera.position = camera.position + (CAMERA_SPEED_SPRINT * direction * deltaTime);
		else camera.position = camera.position + (CAMERA_SPEED * direction * deltaTime);
	}
	camera.position.x = std::clamp(camera.position.x, LOWER_BBOX.x, HIGHER_BBOX.x);
	camera.position.z = std::clamp(camera.position.z, LOWER_BBOX.z, HIGHER_BBOX.z);
	camera.position.y = std::clamp(camera.position.y, LOWER_BBOX.y, HIGHER_BBOX.y);

	//SCALING THE MOON
	if(held(window, GLFW_KEY_M)) scene->moon_scale = scene->moon_scale + MOON_SCALE_SPEED * deltaTime;
	if(held(window, GLFW_KEY_N)) scene->moon_scale = scene->moon_scale - MOON_SCALE_SPEED * deltaTime;

	//ROTATING THE HEART
	if(held(window, GLFW_KEY_Z)) scene->heart_angle = scene->heart_angle - HEART_ROTATION_SPEED * deltaTime;
	if(held(window, GLFW_KEY_X)) scene->heart_angle = scene->heart_angle + HEART_ROTATION_SPEED * deltaTime;

	//EPONA MOVEMENT
	glm::vec3 e_direction = glm::vec3(0.0f);
	if(held(window, GLFW_KEY_UP)) e_direction = e_direction + camera.front;
	if(held(window, GLFW_KEY_DOWN)) e_direction = e_direction - camera.front;
	if(held(window, GLFW_KEY_RIGHT)) e_direction = e_direction + glm::normalize(glm::cross(camera.front, camera.up));
	if(held(window, GLFW_KEY_LEFT)) e_direction = e_direction - glm::normalize(glm::cross(camera.front, camera.up));
	e_direction.y = 0.0f;
	if(glm::length(e_direction) > 0.0f) {
		e_direction = glm::normalize(e_direction);
		scene->epona_pos = scene->epona_pos + (EPONA_SPEED * e_direction * deltaTime);
		scene->epona_last_direction = e_direction;
	}
}


}  // namespace my
