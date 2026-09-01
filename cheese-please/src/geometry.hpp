#pragma once

// --- INCLUSIONS ---
#include <glad/gl.h>    // For the GLint / GLsizei types that glDrawArrays expects.
#include <glm/glm.hpp>  // For `vec3` and `mat4`.
#include <vector>       // Our `Mesh` is a vector of vertices.
                        // What exactly is a "vertex"? See below!


// --- VERTEX & MESH STRUCTURE ---
// Each vertex of each object in this project is composed by two glm 3D vectors.
// The first, stores the vertex's position in 3D space.
// The second, each color channel value (red, green and blue).
//
// This storage method is called INTERLEAVED, because in a list of vertices,
// positional data will be interleaved with color data!
struct Vertex {
	glm::vec3 position;  // attribute 0.
	glm::vec3 color;     // attribute 1.
};

// std::vector<T> isn't a mathematical vector. It is simply an array of
// type T variables of DYNAMIC length.
// The keyword `using` permits us to alias `std::vector<Vertex>` as `Mesh`.
// A briefer and semantically meaningful name.
// Therefore, a `Mesh` variable isn't anything scary. Just a (potentially) long
// list of vertices.
using Mesh = std::vector<Vertex>;  // NOTE: EVERY THREE VERTICES IN A ROW FORM ONE TRIANGLE!

// A `Piece` is the unit of INDEPENDENT MOVEMENT.
// We need to define this `struct` because of a SINGLE ONE of our objects:
// our mousetrap! It has two pieces: its body and its hammer.
// The hammer is able to swing on its own hinge, meanwhile the body stays put.
// All other objects are composed of a single `Piece`!
struct Piece {
	GLint   first;  // index of this part's first vertex in the shared buffer.
	GLsizei count;  // how many vertices belong to it.
};


// --- PUTTING MESHES TOGETHER ---
// Paint a hand-authored table of raw x,y,z floats in one flat colour and append the
// result to `into`. The rat and the mousetrap are modelled this way, vertex by
// vertex; the tables are positions only, and this is what dresses them.
void append(Mesh& into, const std::vector<float>& positions, const glm::vec3& color);

// Add one mesh to another, as it is ...
void append(Mesh& into, const Mesh& part);

// ... or after moving it into place first. This second form is how an object gets
// built out of primitives: `box` and friends each produce their shape at the origin
// at a convenient size, and a matrix carries the copy to where it belongs. It runs
// once, at start-up, so a hundred boxes cost nothing at drawing time.
void append(Mesh& into, const Mesh& part, const glm::mat4& placement);


// --- SIMPLE OBJECTS GENERATION ---
// Simple solids to compose objects out of.

// Create a unit cube centered at the origin
Mesh box(
	const glm::vec3& top ,
	const glm::vec3& side,
	const glm::vec3& base
);


// SOLID OF REVOLUTION
// First, to new types must be created.
// In reverse order:
//	`Profile` is the set of points of a flat surface we want to revolve around the Y axis.
//	`ProfilePoint` is a single point defining such surface.
// NOTE: `ProfilePoint`s are NOT `Vertex`s. They are all assumed coplanar. Therefore, they
//       require only two positional variables, `radius` and `height`.
struct ProfilePoint {
	float     radius;
	float     height;
	glm::vec3 color;
};

using Profile = std::vector<ProfilePoint>;


// Spin a silhouette around the Y axis to make a solid of revolution: the oldest trick
// in the potter's book, and the reason a wine glass takes eight numbers to describe
// instead of two hundred triangles. `segments` is how many flat facets stand in for
// the circle -- more of them is rounder and slower, and few of them on purpose is what
// gives this scene its faceted look.
Mesh revolve(const Profile& profile, int segments);

// The same, but stopping short: `sweep` is how far round to go. A full turn (tau)
// gives a closed solid, and anything less leaves a wedge missing, the way a slice cut
// from a cheese leaves a notch. The two flat faces the cut exposes are filled in and
// painted `cut`, since they are the inside of the object and would not be the same
// colour as its outside.
Mesh revolve(const Profile& profile, int segments,
             float sweep, const glm::vec3& cut);


// --- ONE BUFFER FOR THE WHOLE SCENE ---
// Every object's triangles are poured into a SINGLE vertex buffer, uploaded to the
// GPU once, and never touched again. Each `add` hands back the Piece describing
// where its argument landed.
struct SceneMesh {
	Mesh vertices;  // the whole scene, back to back

	// Append a part and hand back the bookmark to it. Every object arrives here already
	// in the scene's units and centred on whatever it turns about -- see objects.hpp --
	// so there is nothing left to correct on the way in.
	Piece add(const Mesh& part);
};
