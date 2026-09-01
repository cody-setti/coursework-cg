// --- INCLUSIONS ---
#include "scene.hpp"

#include "objects/objects.hpp"     // The five meshes.
#include "primary_transforms.hpp"  // The primary transformations every matrix here is built from.

#include <glm/glm.hpp>             // For `vec2`, `mat4` and glm::normalize.
#include <algorithm>               // For std::clamp and std::min.
#include <cmath>                   // For std::atan2.


namespace {

// --- HOW THE SCENE IS SET OUT ---
// The table's top surface is the plane y = 0, and everything on it is placed at that
// height. x runs left to right across the table, z from the far edge towards us.
// Once the table is turned (see below), its +x axis runs to the right and slightly
// towards us, and its +z axis runs away from us, up the screen.
constexpr float kCheeseX = -0.62f, kCheeseZ =  0.06f;  // the bait, well clear of the trap
constexpr float kCheeseFace =  0.55f;                  // and turned so the slice taken out of it
                                                       // opens towards us, showing off the holes
                                                       // on the two cut faces. (rotation_y turns a
                                                       // bearing DOWNWARDS -- an angle phi ends up
                                                       // at phi minus the argument -- so this sign
                                                       // is the opposite of the one you first write.)
constexpr float kGlassX  =  0.86f, kGlassZ  =  0.12f;  // somebody's glass, right down the table
                                                       // (the mousetrap sits at the origin)

// --- HOW THE TABLE STANDS IN FRONT OF US ---
// The table is not square-on: it is turned a little on the spot and then tipped, so
// that we look over the top of it rather than at its edge. Without that the whole
// scene would arrive as flat silhouettes, there being no lighting to give the solids
// away. Q, E, T and G move those two angles, which live in Scene.
//
// This is NOT a camera, and there is no view or projection matrix in the project.
// Nothing here has an eye position, a target or an up vector. The table is simply an
// object that has been rotated -- exactly what requirements 4 and 7 ask for -- and
// every object standing on the table is built on top of the table's own matrix, the
// way a plate on a tray goes wherever the tray goes.
constexpr float kTableLift  = 0.12f;  // lifted, to sit the whole scene in the middle of
                                      // the window rather than low in it
constexpr float kSceneScale = 0.74f;  // and drawn at this fraction of its modelled size.
                                      // The table is now wide enough that, spun to the
                                      // right angle, its corner would leave the visible
                                      // box: it is the table's DIAGONAL, not its width,
                                      // that has to fit across the window.
constexpr float kDepthSquash = 0.62f; // Coordinates run from -1 to +1 in depth as well,
                                      // and a table this wide overruns that too when it
                                      // is turned side-on. But depth never reaches the
                                      // screen: it only decides which surface is in front
                                      // of which. So the last thing done to every vertex
                                      // is to squash z alone, which brings the far corner
                                      // back inside the box without moving a single pixel
                                      // of the picture. It costs a little of the depth
                                      // buffer's precision, of which there are 24 bits and
                                      // this scene needs a handful.
constexpr float kTurnRate   = 1.20f;  // radians per second, turning the scene
constexpr float kTiltRate   = 0.90f;  // radians per second, tipping it
constexpr float kTiltLow    = 0.08f;  // tipped flat the table would be edge-on and all but
constexpr float kTiltHigh   = 0.65f;  // invisible; tipped much past this its far corner
                                      // leaves the window

// --- HOW FAST EVERYTHING MOVES ---
// Every rate below is per SECOND, and update() multiplies it by the length of the
// frame that just went by. Tie movement to frames instead -- "add 0.001 each time
// round the loop" -- and the scene runs at whatever speed the machine happens to
// draw at, which is faster on a good graphics card and slower on a loaded one.
constexpr float kRatSpeed    = 0.55f;  // table units per second
constexpr float kTrapYawRate = 1.5f;   // radians per second
constexpr float kHammerRate  = 2.5f;   // radians per second
constexpr float kCheeseRate  = 0.7f;   // how fast the piece of cheese grows or shrinks
constexpr float kFlipTime    = 0.9f;   // seconds from take-off to landing
constexpr float kFlipHeight  = 0.30f;  // table units at the top of the leap

// --- WHERE THINGS MAY GET TO ---
// (kRatBelly, how far the rat's middle stands above its paws, and kTrapHinge, the height
// its hammer swings about, are facts about those two meshes and live in objects.hpp.)
constexpr float kRatReachX  = 0.95f;           // it may wander this far along the table ...
constexpr float kRatReachZ  = 0.30f;           // ... and this far across it, before the edge.
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

	// Two pieces out of the one object, because the hammer swings independently of the
	// base it is bolted to.
	const Mousetrap trap = build_mousetrap();

	// Every object arrives ready to place -- objects.hpp promises it, and each object file
	// keeps that promise using its own modelling units -- so there is nothing to correct
	// here and every line reads the same. Each piece is paired with the matrix function
	// that places it as it is poured into the buffer, which leaves no second list anywhere
	// to fall out of step with this one.
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
	// --- THE RAT: TRANSLATION FROM THE KEYBOARD (requirements 4 and 5) ---
	// The arrow keys are read here, once per frame, rather than in the key callback.
	// A callback fires on press, on release, and then over and over while the key
	// auto-repeats -- to a rhythm set by the operating system's typing preferences,
	// which has nothing to do with the scene. Asking "is this key down right now?"
	// each frame is both simpler and smoother: the rat walks while a key is held and
	// stops the moment it is let go.
	//
	// Left and right walk the rat along the table; up and down walk it away from us
	// and back towards us across it, which is what those two arrows look like they
	// should do now that we are looking down at the table from above.
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

	// --- THE RAT: THE FLIP ---
	// One leap is one run of `flip` from 0 up to 1; see rat_transform for the shape
	// that single number is turned into.
	if (scene.flip < 1.0f)
		scene.flip = std::min(scene.flip + seconds / kFlipTime, 1.0f);

	// --- THE MOUSETRAP: ROTATION FROM THE KEYBOARD (requirements 4 and 7) ---
	if (held(window, GLFW_KEY_A)) scene.trap_yaw     -= kTrapYawRate * seconds;
	if (held(window, GLFW_KEY_D)) scene.trap_yaw     += kTrapYawRate * seconds;
	if (held(window, GLFW_KEY_W)) scene.hammer_angle += kHammerRate  * seconds;
	if (held(window, GLFW_KEY_S)) scene.hammer_angle -= kHammerRate  * seconds;

	// Shut flat against the base at 0, standing straight up and over at half a turn.
	// Past either end the hammer would swing down through its own base.
	scene.hammer_angle = std::clamp(scene.hammer_angle, 0.0f, kTau * 0.5f);

	// --- THE CHEESE: SCALE FROM THE KEYBOARD (requirements 4 and 6) ---
	// How big a piece of cheese to leave out as bait. A crumb is safer; a whole wedge
	// is more tempting.
	if (held(window, GLFW_KEY_Z)) scene.cheese_size -= kCheeseRate * seconds;
	if (held(window, GLFW_KEY_X)) scene.cheese_size += kCheeseRate * seconds;
	scene.cheese_size = std::clamp(scene.cheese_size, kCheeseMin, kCheeseMax);

	// --- THE TABLE: TURNING THE WHOLE SCENE (requirements 4 and 7) ---
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
	GLFWwindow*      window  ,  // The window the key was pressed in.
	int              key     ,  // Which key, as a GLFW token (e.g. GLFW_KEY_P).
	[[maybe_unused]] int scancode,  // Where the key physically sits on the board. We work
	                                // in tokens, not positions, so this goes unread.
	int              action  ,  // GLFW_PRESS, GLFW_REPEAT or GLFW_RELEASE.
	[[maybe_unused]] int mods   // Which of Shift/Ctrl/Alt were down. Unread as well.
) {
	// Only the keys that should act ONCE per press live here. Dropping REPEAT and
	// RELEASE is what stops a held-down 'p' from strobing the wireframe.
	if (action != GLFW_PRESS) return;

	// Retrieve the scene main() attached to the window when it registered us.
	Scene* scene = static_cast<Scene*>(glfwGetWindowUserPointer(window));
	if (scene == nullptr) return;

	switch (key) {
	case GLFW_KEY_P:  // Requirement 9: show or hide the polygon mesh.
		// Only the intention is recorded here. The glPolygonMode call that acts on
		// it is made by the render loop, so that every OpenGL call in the program
		// happens in one place, on the thread that owns the context.
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


// --- ONE TRANSFORMATION MATRIX PER OBJECT (requirement 3) ---
// Each function below builds one object's matrix out of nothing but the primary
// transformations from primary_transforms.hpp. They are the heart of the project,
// so they are gathered here rather than scattered through the drawing code.
//
// Every product reads RIGHT TO LEFT: the rightmost matrix is applied to the vertex
// first. And every object that stands on the table begins with the table's own
// matrix, because moving the table has to move everything on it.

// The table is turned on the spot and then tipped towards us, which is what lets the
// solids in the scene be seen as solids. See the note beside kSceneScale: this is the
// table's own rotation, not a camera.
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

	return table_transform(scene)
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
