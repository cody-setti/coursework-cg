// --- CAMERA ---
// A flying eye, of adjustable FoV, and the View (V) and Projection (P) matrices it amounts to.
// Aspect ratio is purposefully not stored: `main.cpp` passes it to `projMatrix()`, according to the window's (re)size.

#pragma once                              // `#include` literally pastes a file's text in place.
                                          // `#pragma once` protects `camera.hpp` from being double pasted.

#include <cmath>                          // C Mathematics: `std::atan2()`.

#include <glm/glm.hpp>                    // GL Mathematics: provides GLSL-compatible vector and matrix types.
#include <glm/ext/matrix_transform.hpp>   // e.g.: `translate()`, `lookAt()`.
#include <glm/ext/matrix_clip_space.hpp>  // e.g.: `perspective()`.

namespace cam {

// TYPES
struct Camera {
	glm::vec3 eye  ;  // Eye position in world coordinates.
	
	// Where the eye faces, as two angles, in radians.
	// These are chosen over a `front` vector, because
	// mouse Δx and Δy update them simply by addition.
	float     yaw  ;  // Heading along the ground, like a compass: 0 faces +x; growing turns towards +z.
	float     pitch;  // Height above (+) / below (-) the horizon, in whichever direction `yaw` faces.
	// ...i.e.: from the eye's point of view, `yaw` comes first, then `pitch` (rotations don't commute!).
	// OBS.: Around the world's FIXED axes, the same direction is reached performing pitch then yaw (opposite order).
	
	float     fov  ;  // Vertical field of view (radians).
};


// CONSTANTS
constexpr float     TAU         = 6.28318530718f             ;  // τ: one full turn, in radians.

constexpr glm::vec3 WORLD_UP    = glm::vec3(0.0f, 1.0f, 0.0f);
// At a pitch of ±0.25τ, `front` would be parallel to `WORLD_UP`: making `right` (their cross product) undefined.
constexpr float     PITCH_LIMIT =   0.249f  * TAU            ;
constexpr float     FOV_MIN     =   0.03f   * TAU            ;
constexpr float     FOV_MAX     =   0.25f   * TAU            ;
constexpr float     NEAR_PLANE  =   0.1f                     ;
constexpr float     FAR_PLANE   = 100.0f                     ;


// FUNCTIONS
	// CREATE CAMERA
inline cam::Camera createCamera(glm::vec3 eye, glm::vec3 target) {
	cam::Camera camera;
	
	camera.eye = eye;
	
	const glm::vec3 front = glm::normalize(target - eye)                        ;
	camera.yaw   = std::atan2(front.z, front.x)                                 ;
	camera.pitch = glm::asin(front.y)                                           ;
	camera.pitch = glm::clamp(camera.pitch, -cam::PITCH_LIMIT, cam::PITCH_LIMIT);
	
	camera.fov = cam::FOV_MAX;
	
	return camera;
}


	// DERIVE CAMERA INFO
// Determines the direction (unit vector) the eye faces based off its yaw and pitch.
inline glm::vec3 frontOf(const cam::Camera& camera) {  // OBS.: `&` is to pass the camera by reference.
                                                       //       `const` promises the function won't change it.
	return glm::vec3(
		glm::cos(camera.pitch) * glm::cos(camera.yaw),
		glm::sin(camera.pitch)                       ,
		glm::cos(camera.pitch) * glm::sin(camera.yaw)
	);
}

inline glm::mat4 viewMatrix(const cam::Camera& camera) {
	return glm::lookAt(
		camera.eye                       ,
		camera.eye + cam::frontOf(camera),
		cam::WORLD_UP
	);
}

// OBS.: `aspect` (framebuffer width/height) comes from the window, not from the camera.
inline glm::mat4 projMatrix(const cam::Camera& camera, float aspect) {
	return glm::perspective(
		camera.fov     ,
		aspect         ,
		cam::NEAR_PLANE,
		cam::FAR_PLANE
	);
}


	// CHANGE CAMERA PARAMETERS
// Moves the eye: `dFwd` towards where it faces, `dSide` to its right.
inline void fly(cam::Camera& camera, float dFwd, float dSide) {
	const glm::vec3 front = cam::frontOf(camera)                            ;
	const glm::vec3 right = glm::normalize(glm::cross(front, cam::WORLD_UP));
	
	camera.eye += (dFwd * front) + (dSide * right);
}

inline void confine(cam::Camera& camera, glm::vec3 eyeMin, glm::vec3 eyeMax) {
	camera.eye = glm::clamp(camera.eye, eyeMin, eyeMax);
}

inline void turn(cam::Camera& camera, float dYaw, float dPitch) {
	camera.yaw   += dYaw                                                         ;
	camera.pitch += dPitch                                                       ;
	// At ±0.25τ, `front` would be parallel to `WORLD_UP`: making `right` undefined.
	camera.pitch  = glm::clamp(camera.pitch, -cam::PITCH_LIMIT, cam::PITCH_LIMIT);
}

inline void zoom(cam::Camera& camera, float dFov) {
	camera.fov += dFov                                              ;
	camera.fov  = glm::clamp(camera.fov, cam::FOV_MIN, cam::FOV_MAX);
}

}  // namespace cam
