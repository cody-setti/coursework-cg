// --- INPUT ---
// Every keyboard and mouse read: what they CHANGE in the window, scene, camera and rasterization.

#pragma once                              // `#include` literally pastes a file's text in place.
                                          // `#pragma once` protects `input.hpp` from being double pasted.

#include <cmath>                          // C Mathematics: `std::atan2()`.

#include <glad/gl.h>                      // GLAD: loads driver-specific OpenGL function pointers at runtime.
                                          // Must be included before GLFW.

#include <GLFW/glfw3.h>                   // GL FrameWork: reads `window`'s keyboard, mouse cursor and mouse wheel state/events.

#include <glm/glm.hpp>                    // GL Mathematics: provides GLSL-compatible vector and matrix types.

#include "world/scene.hpp"                // `scn::Scene`, `scn::EYE_MIN`, `scn::EYE_MAX`.
#include "world/camera.hpp"               // `cam::Camera`, `cam::turn()`, `cam::fly()`, `cam::confine()`, `cam::zoom()`.


namespace input {

// TYPES
struct Carryover {
	bool       pPressed                                  = false          ;
	glm::dvec2 cursor                                    = glm::dvec2(0.0);  // Cursor position in the previous frame, in pixels.
	                                                                         // Δx and Δy are used to update the yaw and pitch, respectively.
	double     scrollY                                   = 0.0            ;
	
	bool       ambientKeyPressed                         = false          ;
	bool       pLightKeysPressed[scn::POINT_LIGHT_COUNT] = {}             ;  // `{}` sets them all to false.
};

// CONSTANTS
constexpr float TAU               = 6.28318530718f;  // τ: one full turn, in radians.

constexpr float NAVI_SPEED        = 9.0f          ;  // World units per second.

constexpr float REFLECTANCE_SPEED = 3.0f          ;
constexpr float REFLECTANCE_MAX   = 6.0f          ;

constexpr float MOUSE_SENSITIVITY = 0.0003f * TAU ;
constexpr float FLY_SPEED         = 3.0f          ;
constexpr float FLY_BOOST         = 3.0f          ;
constexpr float ZOOM_SENSITIVITY  = 0.005f  * TAU ;


// FUNCTIONS

// Keys are either up or down: GLFW stores that state, so we poll it.
// The mouse, however, only reports changes (e.g.: "moved 3 px", "turned 1 notch"), so a running
// total only exists if someone keeps it:
//
// > The cursor's total has a clear meaning: it is a spot on the screen. Therefore, GLFW keeps
//   it and we can just poll it when needed.
//
// > A wheel's total means nothing globally: "+4317 notches since boot". A notch's semantics
//   depend on WHO receives it (e.g.: a text editor or the camera of a game).
//   So GLFW keeps no state for the scroll wheel: it is our responsibility to define how notch
//   events are handled each frame.
//   `input::scrollCallback` collects them and `input::processInput` gives them meaning, since
//   WE know our receiver is the camera's FoV.
//   During `glfwPollEvents()`, GLFW calls `input::scrollCallback` once per notch queued since
//   the last poll.
inline void scrollCallback(GLFWwindow* window, double /*dx*/, double dy) {
	input::Carryover* const carry = (input::Carryover*)glfwGetWindowUserPointer(window);
	(*carry).scrollY += dy;
	// `carry` (declared in main.cpp) outlives the render loop. Therefore, following this pointer is safe.
}

inline void setUpInput(GLFWwindow* window, input::Carryover& carry) {
	glfwSetWindowUserPointer(window, &carry)                  ;  // `carry` is now part of `window`'s state.
	
	glfwSetInputMode(
		window              ,
		GLFW_CURSOR         ,  // ARG.: Configure cursor's mode.
		GLFW_CURSOR_DISABLED   // ARG.: Disabled mode...
		                       // > makes the cursor invisible
		                       // > holds it inside the window
		                       // > assigns it a virtual, unlimited position,
		                       //   as if the window were infinitely wide and tall.
	);
	glfwGetCursorPos(window, &carry.cursor.x, &carry.cursor.y);
	
	glfwSetScrollCallback(window, input::scrollCallback)      ;
}

inline void processInput(
	GLFWwindow*       window   ,
	input::Carryover& carry    ,
	float             dt       ,
	scn::Scene&       scene    ,
	cam::Camera&      camera   ,
	bool&             wireframe
) {
	// ESCAPE WINDOW
	if ( glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS ) glfwSetWindowShouldClose(window, GLFW_TRUE);
	
	// ROTATE CAMERA
	glm::dvec2 cursor                                        ;
	glfwGetCursorPos(window, &cursor.x, &cursor.y)           ;
	// OBS.: Not multiplied by `dt`: the mouse's movement already IS a distance, not a speed.
	const float dx = (float)(      cursor.x - carry.cursor.x);  // Cursor position is measured from   ┌─►
	const float dy = (float)(carry.cursor.y -       cursor.y);  // the top-left corner of the screen. ▼  
	carry.cursor = cursor;
	
	const float dYaw   = input::MOUSE_SENSITIVITY * dx;
	const float dPitch = input::MOUSE_SENSITIVITY * dy;
	cam::turn(camera, dYaw, dPitch)                   ;
	
	// TRANSLATE CAMERA
	float flyStep = input::FLY_SPEED * dt                                                    ;
	if ( glfwGetKey(window, GLFW_KEY_LEFT_SHIFT ) == GLFW_PRESS ) flyStep *= input::FLY_BOOST;
	
	glm::vec2 flyDir = glm::vec2(0.0f);  // x: W/S, y: A/D.
	if ( glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ) flyDir.x += 1.0f;
	if ( glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ) flyDir.y -= 1.0f;
	if ( glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ) flyDir.x -= 1.0f;
	if ( glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS ) flyDir.y += 1.0f;
	
	if ( flyDir != glm::vec2(0.0f) ) {
		flyDir = glm::normalize(flyDir);
		cam::fly(camera, flyStep*flyDir.x , flyStep*flyDir.y);
	}
	cam::confine(camera, scn::EYE_MIN, scn::EYE_MAX);
	
	// ZOOM CAMERA
	// Scrolling up (+) must narrow the FoV: hence the minus sign.
	const float dFov          = -input::ZOOM_SENSITIVITY * (float)carry.scrollY;
	            carry.scrollY = 0.0                                            ;
	cam::zoom(camera, dFov)                                                    ;
	
	// TRANSLATE NAVI
	const float naviStep = input::NAVI_SPEED * dt;
	glm::vec2   naviDir  = glm::vec2(0.0f)       ;
	if ( glfwGetKey(window, GLFW_KEY_UP   ) == GLFW_PRESS ) naviDir.x += 1.0f;
	if ( glfwGetKey(window, GLFW_KEY_DOWN ) == GLFW_PRESS ) naviDir.x -= 1.0f;
	if ( glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS ) naviDir.y += 1.0f;
	if ( glfwGetKey(window, GLFW_KEY_LEFT ) == GLFW_PRESS ) naviDir.y -= 1.0f;
	
	if ( naviDir != glm::vec2(0.0f) ) {
		naviDir               = glm::normalize(naviDir);
		
		// Express movement coordinates in terms of world reference.
		const glm::vec3 front = glm::vec3( glm::cos(camera.yaw), 0.0f, glm::sin(camera.yaw));
		const glm::vec3 right = glm::vec3(-glm::sin(camera.yaw), 0.0f, glm::cos(camera.yaw));
		
		const glm::vec3 move = naviDir.x*front + naviDir.y*right;
		
		scene.navi.position  += naviStep * move;
		scene.navi.angles.y   = std::atan2(move.x, move.z);  // Make navi face the direction she moves in.
	}
	
	// TOGGLE WIREFRAME
	const bool pPressed = glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS;
	if ( pPressed && !carry.pPressed ) wireframe = !wireframe;  // Toggle once per press, not once per frame held.
	carry.pPressed = pPressed;
	
	
	// TOGGLE LIGHTS
	const bool ambientKeyPressed = glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS;
	if ( ambientKeyPressed && !carry.ambientKeyPressed ) scene.ambientOn = !scene.ambientOn;  // Toggle once per press, not once per frame held.
	carry.ambientKeyPressed = ambientKeyPressed;
	
	for ( int i = 0; i < scn::POINT_LIGHT_COUNT; i++ ) {
		const bool pLightKeyPressed = glfwGetKey(window, GLFW_KEY_2 + i) == GLFW_PRESS;
		if ( pLightKeyPressed && !carry.pLightKeysPressed[i] ) scene.pLightsOn ^= (1u << i);  // XOR flips only bit i.
		carry.pLightKeysPressed[i] = pLightKeyPressed;
	}
	
	// SCALE REFLECTIONS
	const float reflectanceStep = input::REFLECTANCE_SPEED * dt;
	if ( glfwGetKey(window, GLFW_KEY_R           ) == GLFW_PRESS ) scene.ambientScale  += reflectanceStep;
	if ( glfwGetKey(window, GLFW_KEY_F           ) == GLFW_PRESS ) scene.ambientScale  -= reflectanceStep;
	if ( glfwGetKey(window, GLFW_KEY_E           ) == GLFW_PRESS ) scene.diffuseScale  += reflectanceStep;
	if ( glfwGetKey(window, GLFW_KEY_Q           ) == GLFW_PRESS ) scene.diffuseScale  -= reflectanceStep;
	if ( glfwGetKey(window, GLFW_KEY_SPACE       ) == GLFW_PRESS ) scene.specularScale += reflectanceStep;
	if ( glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ) scene.specularScale -= reflectanceStep;
	scene.ambientScale  = glm::clamp(scene.ambientScale , 0.0f, input::REFLECTANCE_MAX);
	scene.diffuseScale  = glm::clamp(scene.diffuseScale , 0.0f, input::REFLECTANCE_MAX);
	scene.specularScale = glm::clamp(scene.specularScale, 0.0f, input::REFLECTANCE_MAX);
}

}  // namespace input
