#pragma once
// --- CAMERA ---
// Keeping the camera's state and turning it into a View matrix (which keys move it is decided in "input.hpp").
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/glm.hpp>                    // GL Mathematics: `glm::vec3`, `glm::mat4` and `normalize()`.
#include <glm/ext/matrix_transform.hpp>   // `lookAt()`.
#include <glm/trigonometric.hpp>


namespace my {

// Everything needed to build the View matrix.
struct Camera {
	glm::vec3 position;  // Eye  : where the camera is (in world space).
	glm::vec3 front   ;  // Unit vector: the direction it looks at.
	glm::vec3 up      ;  // Which world direction is "up" for the camera.
	double pitch = 0.0		  ;
	double yaw = -90.0		  ;
	double roll = 0.0		  ;
};

//Calculates the normalized front vector based on Euler Angles (pitch, yaw and roll) roll is not actually used in the code at all.
inline void calculateFront(my::Camera& camera) {
	camera.front.x = glm::cos(glm::radians(camera.yaw)) * glm::cos(glm::radians(camera.pitch));
	camera.front.y = glm::sin(glm::radians(camera.pitch));
	camera.front.z = glm::sin(glm::radians(camera.yaw)) * glm::cos(glm::radians(camera.pitch));
	camera.front = glm::normalize(camera.front);
};

// Builds the View matrix: moves the world so that the camera sits at the origin, looking down -Z.
inline glm::mat4 viewMatrix(const my::Camera& camera) {
	return glm::lookAt(
		camera.position               ,  // Eye.
		camera.position + camera.front,  // Center: a point straight ahead of the eye.
		camera.up
	);
}

// TODO: functions that move and turn the camera, for `my::updateInput()` to call
//       (requirement 9: keep the camera within the sky's and the floor's limits).

}  // namespace my
