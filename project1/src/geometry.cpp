// --- INCLUSIONS ---
#include "geometry.hpp"

#include "primary_transforms.hpp"  // For kTau, when swinging a profile around the axis.

#include <cassert>                 // For the coplanarity check in quad().
#include <glm/glm.hpp>             // For `vec3`, `vec4` and `mat4`.
#include <cmath>                   // For std::cos, std::sin and std::fabs.
#include <vector>


// --- PUTTING MESHES TOGETHER ---

// Walk the flat float table three at a time, turning each triple into a vertex and
// giving all of them the same colour.
void append(Mesh& into, const std::vector<float>& positions, const glm::vec3& color) {
	into.reserve(into.size() + positions.size() / 3);
	for (std::size_t i = 0; i + 2 < positions.size(); i += 3)
		into.push_back(Vertex{
			glm::vec3(positions[i], positions[i + 1], positions[i + 2]),
			color
		});
}


void append(Mesh& into, const Mesh& part) {
	into.insert(into.end(), part.begin(), part.end());
}


void append(Mesh& into, const Mesh& part, const glm::mat4& placement) {
	into.reserve(into.size() + part.size());
	for (const Vertex& vertex : part)
		into.push_back(Vertex{
			// `w = 1` marks this as a POINT rather than a direction, so the
			// placement's translation column actually applies to it. The result is
			// cut back down to three floats: nothing in this project needs the
			// fourth once the multiplication is done.
			glm::vec3(placement * glm::vec4(vertex.position, 1.0f)),
			vertex.color
		});
}


// --- PRIMITIVES ---
namespace {  // An ANONYMOUS namespace. Everything inside it is private to this file.
	void quad(  // Build a quadrilateral surface.
	            // This function is auxiliary to `box()`.
		// The `Mesh` to append to. A reference, so the caller's mesh grows.
		Mesh& into,
		// Corners of the four-sided surface, in order around its edge.
		const glm::vec3& a,
		const glm::vec3& b,
		const glm::vec3& c,
		const glm::vec3& d,
		// The surface's color.
		const glm::vec3& color
	) {
		// ATTENTION: If a, b, c and d aren't COPLANAR, the surface geometry depends on the split!
		// split a–c:  a ______ b        split b–d:  a _______ b
		//             |ridge  /|                    |\  valley|
		//             |     /  |                    |  \      |
		//             d_______ c                    d _______ c
		// Therefore...
		// Assert that the volume of the tetrahedron spanned by vertices a,b,c and d is practically zero.
		// Why? That is mathematically equivalent to them being COPLANAR.
		assert(std::fabs(glm::dot(d - a, glm::cross(b - a, c - a))) < 1e-4f
		       && "quad(): the four corners are not coplanar");
	
		// Make the first triangular surface.
		into.push_back(Vertex{ a, color });
		into.push_back(Vertex{ b, color });
		into.push_back(Vertex{ c, color });
	
		// Make the second triangular surface.
		into.push_back(Vertex{ a, color });
		into.push_back(Vertex{ c, color });
		into.push_back(Vertex{ d, color });
	}
}


Mesh box(  // Returns a `Mesh` describing a unit box centered at the origin.
	// The box's top, sides and base are given distinct colors,
	// to create the ILLUSION of lighting.
	const glm::vec3& top ,
	const glm::vec3& side,
	const glm::vec3& base
) {
	const float h = 0.5f;  // half an edge, so the whole edge measures 1.
	Mesh mesh;             // initialize the `Mesh` that will store the unit box.
	mesh.reserve(36);      // 6 faces of 6* vertices each = 36 vertices.
	                       // *: 2 of the 4 vertices are repeated when using `quad`!

	quad(mesh, {-h, -h,  h}, { h, -h,  h}, { h,  h,  h}, {-h,  h,  h}, side);  // front  (+Z)
	quad(mesh, { h, -h, -h}, {-h, -h, -h}, {-h,  h, -h}, { h,  h, -h}, side);  // back   (-Z)
	quad(mesh, { h, -h,  h}, { h, -h, -h}, { h,  h, -h}, { h,  h,  h}, side);  // right  (+X)
	quad(mesh, {-h, -h, -h}, {-h, -h,  h}, {-h,  h,  h}, {-h,  h, -h}, side);  // left   (-X)
	quad(mesh, {-h,  h,  h}, { h,  h,  h}, { h,  h, -h}, {-h,  h, -h}, top );  // top    (+Y)
	quad(mesh, {-h, -h, -h}, { h, -h, -h}, { h, -h,  h}, {-h, -h,  h}, base);  // bottom (-Y)

	return mesh;
}

// BEWARE: Cool function OVERLOADING ahead!
//         Which function to use?! Picked at compile time by looking at the passed arguments!
Mesh revolve(
	const Profile& profile,
	int segments
) {
	// A whole turn closes the solid on itself, so there is no cut and no colour for one.
	return revolve(
		profile        ,
		segments       ,
		kTau           ,  // A complete revolution.
		glm::vec3(0.0f)   // No cut, so the value of this color is irrelevant.
	);
}


namespace {  // Another ANONYMOUS namespace: these two helpers serve `revolve()` alone.
	// Where a profile point lands once it has been swung `angle` radians around the
	// Y axis: `radius` measures outwards from the axis, `height` along it.
	glm::vec3 point_at(const ProfilePoint& point, float angle) {
		return glm::vec3(
			point.radius * std::cos(angle),
			point.height                  ,
			point.radius * std::sin(angle)
		);
	}

	// The same, dressed up as a finished `Vertex` in the profile point's own color.
	Vertex on_ring(const ProfilePoint& point, float angle) {
		return Vertex{
			point_at(point, angle),
			point.color
		};
	}
}


Mesh revolve(
	const Profile& profile,
	int segments          ,
	float sweep           ,  // how far round to go, in radians
	const glm::vec3& cut     // color of the faces the missing wedge exposed.
	                         // This creates the ILLUSION of lighting and/or depth.
) {
	Mesh mesh;
	// GUARD CLAUSE: Revolving a single `ProfilePoint` would produce a circular curve, not a surface!
	//               Try approximating a circle with two segments... impossible!
	//               At the very least we approximate one by a triangle (3 segments).
	if (profile.size() <= 1 || segments <= 2) return mesh;

	// --- THE CURVED SURFACE ---
	for (int segment = 0; segment < segments; ++segment) {
		const float a0 = sweep * static_cast<float>(segment    ) / static_cast<float>(segments);
		const float a1 = sweep * static_cast<float>(segment + 1) / static_cast<float>(segments);

		// Each neighbouring pair of profile points sweeps out one band of the surface.
		for (std::size_t i = 0; i + 1 < profile.size(); ++i) {
			const ProfilePoint& lower = profile[i];
			const ProfilePoint& upper = profile[i + 1];

			const Vertex l0 = on_ring(lower, a0), l1 = on_ring(lower, a1);
			const Vertex u0 = on_ring(upper, a0), u1 = on_ring(upper, a1);

			// Right on the axis a whole ring collapses to a single point, so the band
			// is a triangle rather than a four-sided strip. Emitting the strip anyway
			// would leave a degenerate, invisible triangle behind -- harmless when
			// filled, but a stray spoke once the wireframe is switched on.
			if (lower.radius == 0.0f) {
				mesh.push_back(l0); mesh.push_back(u0); mesh.push_back(u1);
				continue;
			}
			if (upper.radius == 0.0f) {
				mesh.push_back(l0); mesh.push_back(l1); mesh.push_back(u0);
				continue;
			}

			mesh.push_back(l0); mesh.push_back(l1); mesh.push_back(u1);
			mesh.push_back(l0); mesh.push_back(u1); mesh.push_back(u0);
		}
	}

	// --- THE TWO CUT FACES ---
	// A sweep that stops short leaves the solid open along two flat faces, one at each
	// end of the arc. Each is the profile itself, closed back on the axis, filled in
	// by a fan of triangles from its first point, with the one proviso that the profile
	// has no dents in it.
	if (sweep < kTau - 0.0001f) {
		for (std::size_t i = 1; i + 1 < profile.size(); ++i) {
			mesh.push_back(Vertex{ point_at(profile[0]    , 0.0f), cut });
			mesh.push_back(Vertex{ point_at(profile[i + 1], 0.0f), cut });
			mesh.push_back(Vertex{ point_at(profile[i]    , 0.0f), cut });

			mesh.push_back(Vertex{ point_at(profile[0]    , sweep), cut });  // the far face,
			mesh.push_back(Vertex{ point_at(profile[i]    , sweep), cut });  // wound the other
			mesh.push_back(Vertex{ point_at(profile[i + 1], sweep), cut });  // way round
		}
	}

	return mesh;
}


// --- ONE BUFFER FOR THE WHOLE SCENE ---

Piece SceneMesh::add(const Mesh& part) {
	const Piece piece{
		static_cast<GLint>(vertices.size()),  // it starts where the buffer currently ends
		static_cast<GLsizei>(part.size())
	};
	append(vertices, part);
	return piece;
}
