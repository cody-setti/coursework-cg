#pragma once

// --- INCLUSIONS ---
#include <glad/gl.h>     // Must come before GLFW: it is the one that defines the
                         // OpenGL symbols, and GLFW checks whether they are there.
#include <GLFW/glfw3.h>  // The scene reads the keyboard, so it needs the window.
#include <glm/glm.hpp>

#include <array>         // For the fixed-size table of parts below.

#include "geometry.hpp"


// --- ONE DRAWABLE PART OF THE SCENE ---
// A Piece says WHERE an object's triangles sit in the shared vertex buffer. The matrix
// function beside it says WHERE THE OBJECT GOES this frame. Those two facts are useless
// apart and are always wanted together, so they are kept together -- which is what lets
// the render loop in main.cpp be a loop at all, knowing nothing about which objects the
// scene happens to contain.
//
// Held as two separate lists instead -- a field per object here, a draw call per object
// over there -- they have to be kept in step by hand, and forgetting an object in the
// second list makes it vanish from the picture with nothing anywhere to say so.
struct Scene;  // named here so that the function pointer below can mention it

struct Part {
	Piece     piece;                        // where its triangles are, and
	glm::mat4 (*transform)(const Scene&);   // how to place them (the functions at the foot of this file)
};


// --- THE SCENE ---
// Everything the program has to remember from one frame to the next, in one place
// and with a sensible starting value written beside each field.
//
// It is a plain value living on main's stack. The key callback reaches it through
// glfwSetWindowUserPointer rather than through a global variable: GLFW hands every
// callback the window it fired on, and the window can be made to carry a pointer of
// our choosing.
struct Scene {
	// --- THE SIX INDEPENDENTLY MOVING PARTS ---
	// Five objects, but six parts: the mousetrap's hammer swings on its own hinge and so
	// needs a matrix of its own. The size is fixed on purpose -- add a seventh part and
	// build_scene stops compiling, which is a far better way to hear about it than an
	// object quietly missing from the screen.
	std::array<Part, 6> parts{};

	// --- THE RAT ---
	// Where it stands on the tabletop, and which way it last walked. Both are (x, z)
	// pairs -- the two directions there are to walk in on a horizontal table. GLM has
	// no "xz" vector, so these are vec2s whose `.y` member means the table's z axis.
	glm::vec2 rat_position{ 0.38f, -0.24f }; // just right of the trap, along the near edge
	glm::vec2 rat_heading { -1.0f, 0.00f };  // facing the trap, which is the way it was modelled
	float     flip        { 1.0f };          // 0 -> 1 over one leap; 1 means "not leaping",
	                                         // which is why it starts there.

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
	bool wireframe{ false };     // requirement 9: is the polygon mesh on show?
};


// --- SETTING THE SCENE UP ---
// Build all five objects, pour them into one vertex buffer, and pair each piece with
// the matrix function that places it.
Scene build_scene(SceneMesh& mesh);


// --- MOVING IT ALONG ---
// Advance the scene by `seconds`, reading whichever keys are being held down.
void update(Scene& scene, GLFWwindow* window, float seconds);

// The keys that act once per press rather than for as long as they are held.
// The signature is dictated by GLFW.
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);


// --- ONE TRANSFORMATION MATRIX PER OBJECT (requirement 3) ---
glm::mat4 table_transform       (const Scene& scene);
glm::mat4 rat_transform         (const Scene& scene);
glm::mat4 trap_body_transform   (const Scene& scene);
glm::mat4 trap_hammer_transform (const Scene& scene);
glm::mat4 cheese_transform      (const Scene& scene);
glm::mat4 wine_glass_transform  (const Scene& scene);
