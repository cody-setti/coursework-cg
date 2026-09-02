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
void append(Mesh& into, const std::vector<float>& positions, const glm::vec3& color);
void append(Mesh& into, const Mesh& part);
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


// Spin a silhouette around the Y axis to make a solid of revolution.
Mesh revolve(const Profile& profile, int segments);

// The same, but stopping short. This exposes two "cuts", that receive special colering.
Mesh revolve(const Profile& profile, int segments,
             float sweep, const glm::vec3& cut);


// --- ONE BUFFER FOR THE WHOLE SCENE ---
struct SceneMesh {
	Mesh vertices;  // the whole scene, back to back
	Piece add(const Mesh& part);
};
