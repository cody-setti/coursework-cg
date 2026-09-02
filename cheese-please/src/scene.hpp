#pragma once

// --- INCLUSIONS ---
#include <glad/gl.h>     // Must come before GLFW: it is the one that defines the
                         // OpenGL symbols, and GLFW checks whether they are there.
#include <GLFW/glfw3.h>  // The scene reads the keyboard, so it needs the window.
#include <glm/glm.hpp>

#include <array>         // For the fixed-size table of parts below.

#include "geometry.hpp"



struct Scene;  // named here so that the function pointer below can mention it

struct Part {
	Piece     piece;                        // where its triangles are
	glm::mat4 (*transform)(const Scene&);   // function pointer for the transformation matrix of the object
};


// --- THE SCENE ---
// Everything the program has to remember from one frame to the next, in one place
// and with a sensible starting value written beside each field.
//
// Passed it's address to the wnidow so it can be used in key_callback
struct Scene {
	//All the 6 moving parts are stored here (although we have 5 objects, the mousetrap has 2 moving parts)
	std::array<Part, 6> parts{};

	// --- THE RAT ---
	// Position of the rat on the table. first coordinate = x and second coordinate = z
	glm::vec2 rat_position{ 0.38f, -0.24f }; // just right of the trap, along the near edge
	glm::vec2 rat_heading { -1.0f, 0.00f };  // facing the trap, which is the way it was modelled
	float     flip        { 1.0f };          // 0 -> 1 over one leap; 1 means "not leaping", which is why it starts there.

	// --- THE MOUSETRAP ---
	float trap_yaw    { 0.0f };  // how far the whole trap has been turned on the spot
	float hammer_angle{ 0.0f };  // how far its hammer has swung open, in [0, pi]

	// --- THE CHEESE ---
	float cheese_size { 1.0f };  // how big a piece has been put out as bait

	// --- THE TABLE ---
	// How the table is stood in front of us. Everything else in the scene is built on
	// top of the table's matrix, so turning these turns the whole tableful together.
	float table_turn { 0.50f };  // radians turned about the upright axis
	float table_tilt { 0.42f };  // radians tipped towards us, so we look over the top

	// --- WHAT THE VIEWER ASKED FOR ---
	bool wireframe{ false };     // is the polygon mesh on show?
};


// --- SETTING THE SCENE UP ---
// Build all five objects, pour them into one vertex buffer, and pair each piece with the matrix function that places it.
Scene build_scene(SceneMesh& mesh);


// --- MOVING IT ALONG ---
// Advance the scene by `seconds`, reading whichever keys are being held down.
void update(Scene& scene, GLFWwindow* window, float seconds);

// The keys that act once per press rather than for as long as they are held.
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);


// Transformation matrix for each movable part
glm::mat4 table_transform       (const Scene& scene);
glm::mat4 rat_transform         (const Scene& scene);
glm::mat4 trap_body_transform   (const Scene& scene);
glm::mat4 trap_hammer_transform (const Scene& scene);
glm::mat4 cheese_transform      (const Scene& scene);
glm::mat4 wine_glass_transform  (const Scene& scene);
