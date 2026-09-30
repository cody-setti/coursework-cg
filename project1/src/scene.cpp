// --- INCLUSIONS ---
#include "scene.hpp"

#include "objects/objects.hpp"     // The five meshes.
#include "primary_transforms.hpp"  // The primary transformations every matrix here is built from.

#include <glm/glm.hpp>             // For `vec2`, `mat4` and glm::normalize.
#include <algorithm>               // For std::clamp and std::min.
#include <cmath>                   // For std::atan2.


namespace {

// The table's top surface is the plane y = 0, and everything on it is placed at that
// height. x runs left to right across the table, z from the far edge towards us.
// Once the table is turned (see below), its +x axis runs to the right and slightly
// towards us, and its +z axis runs away from us, up the screen.

//Important constants for each object
constexpr float kCheeseX = -0.62f, kCheeseZ =  0.06f;  // Position of the cheese in the scene
constexpr float kCheeseFace =  0.55f;                  // Rotation across Y-axis so the cut is facing us
constexpr float kGlassX  =  0.86f, kGlassZ  =  0.12f;  // Position of the glass



constexpr float kTableLift  = 0.12f;  // lifted, to sit the whole scene in the middle of the window rather than low in it
constexpr float kSceneScale = 0.74f;  // fit the whole scene on screen
constexpr float kDepthSquash = 0.62f; // Squash z direction to fit everything
constexpr float kTurnRate   = 1.20f;  // radians per second, turning the scene
constexpr float kTiltRate   = 0.90f;  // radians per second, tipping it
constexpr float kTiltLow    = 0.08f;  // tipped flat the table would be edge-on and all but
constexpr float kTiltHigh   = 0.65f;  // invisible; tipped much past this its far corner leaves the window

// --- HOW FAST EVERYTHING MOVES ---
// Every rate below is per SECOND, and update() multiplies it by the length of the
// frame that just went by. Tie movement to frames instead of the speed of the machine
constexpr float kRatSpeed    = 0.55f;  // table units per second
constexpr float kTrapYawRate = 1.5f;   // radians per second
constexpr float kHammerRate  = 2.5f;   // radians per second
constexpr float kCheeseRate  = 0.7f;   // how fast the piece of cheese grows or shrinks
constexpr float kFlipTime    = 0.9f;   // seconds from take-off to landing
constexpr float kFlipHeight  = 0.30f;  // table units at the top of the leap

// --- WHERE THINGS MAY GET TO ---
constexpr float kRatReachX  = 0.85f;           // it may wander this far along the table ...
constexpr float kRatReachZ  = 0.65f;           // ... and this far across it, before the edge.
                                               // The table reaches 1.20 by 0.40, and a rat is
                                               // allowed to lean over the edge a little
constexpr float kCheeseMin  = 0.35f;           // nibbled down to a crumb
constexpr float kCheeseMax  = 1.50f;           // or a piece too big to fit past the trap


// Is this key held down at this very moment?
bool held(GLFWwindow* window, int key) {
	return glfwGetKey(window, key) == GLFW_PRESS;
}

}  // namespace


// --- SETTING THE SCENE UP ---
Scene build_scene(SceneMesh& mesh) {
	Scene scene;

	// Two pieces out of the one object, because the hammer swings independently of the base it is bolted to.
	const Mousetrap trap = build_mousetrap();
	//Initialize the scene parts
	scene.parts = {{
		{ mesh.add(build_table())      , table_transform       },
		{ mesh.add(build_rat())        , rat_transform         },
		{ mesh.add(trap.body)          , trap_body_transform   },
		{ mesh.add(trap.hammer)        , trap_hammer_transform },
		{ mesh.add(build_cheese())     , cheese_transform      },
		{ mesh.add(build_wine_glass()) , wine_glass_transform  },
	}};

	return scene;
}


// --- MOVING IT ALONG ---
void update(Scene& scene, GLFWwindow* window, float seconds) {
	// Move the rat. Left and right moves in X-axis and up and down moves in Z-axis
	glm::vec2 step(0.0f, 0.0f);
	if (held(window, GLFW_KEY_LEFT )) step.x -= 1.0f;
	if (held(window, GLFW_KEY_RIGHT)) step.x += 1.0f;
	if (held(window, GLFW_KEY_UP   )) step.y += 1.0f;  // +z leads away from the viewer
	if (held(window, GLFW_KEY_DOWN )) step.y -= 1.0f;

	if (step.x != 0.0f || step.y != 0.0f) {
		// The arrows point at the SCREEN, but the rat walks on the TABLE, and the table
		// may have been turned since. Undoing the table's turn on the key direction is
		// what keeps "left" meaning left on screen however far the scene has been spun.
		// Note what is being turned here: the two numbers that came off the keyboard,
		// not a single vertex. Nothing about this is a camera.
		const float c = std::cos(scene.table_turn);
		const float s = std::sin(scene.table_turn);
		step = glm::vec2(step.x * c - step.y * s, step.x * s + step.y * c);

		// Normalised, so that walking diagonally is not faster than walking straight --
		// two keys held at once would otherwise add up to a step of length sqrt(2).
		scene.rat_heading   = glm::normalize(step);
		scene.rat_position += scene.rat_heading * (kRatSpeed * seconds);

		scene.rat_position.x = std::clamp(scene.rat_position.x, -kRatReachX, kRatReachX);
		scene.rat_position.y = std::clamp(scene.rat_position.y, -kRatReachZ, kRatReachZ);
	}

	// RAT FLIP!!!!
	// One leap is one run of `flip` from 0 up to 1; see rat_transform for the shape
	// that single number is turned into.
	if (scene.flip < 1.0f)
		scene.flip = std::min(scene.flip + seconds / kFlipTime, 1.0f);

	// rotation of the mousetrap
	if (held(window, GLFW_KEY_A)) scene.trap_yaw     -= kTrapYawRate * seconds;
	if (held(window, GLFW_KEY_D)) scene.trap_yaw     += kTrapYawRate * seconds;
	if (held(window, GLFW_KEY_W)) scene.hammer_angle += kHammerRate  * seconds;
	if (held(window, GLFW_KEY_S)) scene.hammer_angle -= kHammerRate  * seconds;

	// Shut flat against the base at 0, standing straight up and over at half a turn.
	// Past either end the hammer would swing down through its own base.
	scene.hammer_angle = std::clamp(scene.hammer_angle, 0.0f, kTau * 0.5f);

	//Scaling the cheese
	if (held(window, GLFW_KEY_Z)) scene.cheese_size -= kCheeseRate * seconds;
	if (held(window, GLFW_KEY_X)) scene.cheese_size += kCheeseRate * seconds;
	scene.cheese_size = std::clamp(scene.cheese_size, kCheeseMin, kCheeseMax);

	//Turning the table
	// Q and E turn the table on the spot; T and G tip it towards Top or Ground. Because
	// every object in the scene begins its matrix with the table's, the whole tableful
	// comes round together -- rat, trap, cheese and glass keeping their places on it.
	if (held(window, GLFW_KEY_Q)) scene.table_turn -= kTurnRate * seconds;
	if (held(window, GLFW_KEY_E)) scene.table_turn += kTurnRate * seconds;
	if (held(window, GLFW_KEY_T)) scene.table_tilt += kTiltRate * seconds;
	if (held(window, GLFW_KEY_G)) scene.table_tilt -= kTiltRate * seconds;

	scene.table_tilt = std::clamp(scene.table_tilt, kTiltLow, kTiltHigh);

	// The turn may run round and round for as long as the key is held, so it is wrapped
	// back into a single revolution. Nothing on screen changes; it just stops the number
	// growing without bound, and with it the gaps between the floats near it.
	scene.table_turn = std::fmod(scene.table_turn, kTau);
}


void key_callback(
	GLFWwindow*      window  , 
	int              key     , 
	[[maybe_unused]] int scancode, 
	int              action  , 
	[[maybe_unused]] int mods //clang screams too much without this tag
) {
	// Only the keys that should act ONCE per press live here. Dropping REPEAT and
	// RELEASE is what stops a held-down 'p' from strobing the wireframe.
	if (action != GLFW_PRESS) return;

	// Retrieve the scene main() attached to the window when it registered us.
	Scene* scene = static_cast<Scene*>(glfwGetWindowUserPointer(window));
	if (scene == nullptr) return;

	switch (key) {
	case GLFW_KEY_P:  // show or hide the polygon mesh.

		scene->wireframe = !scene->wireframe;
		break;

	case GLFW_KEY_F:  // Take off, unless the rat is already in the air.
		if (scene->flip >= 1.0f) scene->flip = 0.0f;
		break;

	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(window, GLFW_TRUE);
		break;

	default:
		break;
	}
}



glm::mat4 table_transform(const Scene& scene) {
	return scaling(1.0f, 1.0f, kDepthSquash)   // applied last, and to depth only
	     * translation(0.0f, kTableLift, 0.0f)
	     * rotation_x(-scene.table_tilt)
	     * rotation_y(scene.table_turn)
	     * scaling(kSceneScale, kSceneScale, kSceneScale);
}


glm::mat4 rat_transform(const Scene& scene) {
	const bool airborne = scene.flip < 1.0f;

	// How high the rat's middle is above the table. A leap adds a parabola: zero at
	// both ends and highest in the middle, which is the arc a thrown object traces.
	float height = kRatBelly;
	if (airborne) {
		const float t = scene.flip;
		height += kFlipHeight * 4.0f * t * (1.0f - t);
	}

	// The rat was modelled facing along -X, so this is the turn that brings its nose
	// round to the direction it is walking in. Since rat_heading remembers the last
	// direction walked, a rat that stops keeps facing where it was headed rather than
	// snapping back to a default.
	const float yaw = std::atan2(scene.rat_heading.y, -scene.rat_heading.x);

	// A full forward somersault on the way over. The rat lies along X with its width
	// running along Z, so Z is the axis through its shoulders -- the one a gymnast
	// turns about in a front flip. Turning it the positive way sends the nose down
	// and under first, which is what makes it a front flip rather than a back one.
	//
	// The somersault is applied BEFORE the yaw (it is further right in the product),
	// so it happens in the rat's own frame: whichever way the rat has turned to face,
	// it still goes over its own nose rather than sideways.
	const float somersault = airborne ? kTau * scene.flip : 0.0f;

	return table_transform(scene) //the position of the rat must follow the table movement
	     * translation(scene.rat_position.x, height, scene.rat_position.y)
	     * rotation_y(yaw)
	     * rotation_z(somersault);
}


// The trap turns on the spot. Its base was modelled straddling the origin, so
// turning about the origin already turns it about its own middle: no correction
// needed.
glm::mat4 trap_body_transform(const Scene& scene) {
	return table_transform(scene) * rotation_y(scene.trap_yaw);
}


// The hammer follows whatever turn the base has been given, and swings on top of it.
// Its hinge does NOT pass through the origin -- it lies along y = kTrapHinge -- so
// it needs the standard recipe for rotating about an axis that sits somewhere else:
// carry the axis to the origin, do the rotation there, carry it back.
//
// Read right to left: lower the hinge onto the origin, swing the hammer, turn with
// the base, lift the hinge back into place.
glm::mat4 trap_hammer_transform(const Scene& scene) {
	return table_transform(scene)
	     * translation(0.0f,  kTrapHinge, 0.0f)
	     * rotation_y(scene.trap_yaw)
	     * rotation_z(scene.hammer_angle)
	     * translation(0.0f, -kTrapHinge, 0.0f);
}


// The cheese is scaled last of all -- rightmost, so applied first -- which means it
// grows about its own origin. That origin sits in the middle of its base, on the
// tabletop, so a bigger piece of cheese rises off the table instead of sinking half
// way into it.
glm::mat4 cheese_transform(const Scene& scene) {
	return table_transform(scene)
	     * translation(kCheeseX, 0.0f, kCheeseZ)
	     * rotation_y(kCheeseFace)
	     * scaling(scene.cheese_size, scene.cheese_size, scene.cheese_size);
}


// The glass just stands there, but it stands somewhere, and where it stands depends
// on where the table is: hence a matrix of its own rather than a shortcut.
glm::mat4 wine_glass_transform(const Scene& scene) {
	return table_transform(scene) * translation(kGlassX, 0.0f, kGlassZ);
}
