// --- INCLUSIONS ---
#include "geometry.hpp"

#include "primary_transforms.hpp"  // For kTau, when swinging a profile around the axis.

#include <glm/glm.hpp>             // For `vec3`, `vec4` and `mat4`.
#include <cmath>                   // For std::cos and std::sin.
#include <vector>


namespace {

// Two triangles making up one flat four-sided face. Every primitive below is built
// out of this, which is why none of them has to spell out a triangle by hand.
void quad(Mesh& into,
          const glm::vec3& a, const glm::vec3& b,
          const glm::vec3& c, const glm::vec3& d,
          const glm::vec3& color) {
	into.push_back(Vertex{ a, color });
	into.push_back(Vertex{ b, color });
	into.push_back(Vertex{ c, color });

	into.push_back(Vertex{ a, color });
	into.push_back(Vertex{ c, color });
	into.push_back(Vertex{ d, color });
}

}  // namespace


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

// A box one unit along each edge, centred on the origin, so that scaling it by
// (w, h, d) gives a box of exactly those dimensions and translating it by p puts its
// middle exactly at p. Corners are named from the box's own point of view: the top
// face is the one at +Y.
Mesh box(const glm::vec3& top, const glm::vec3& side, const glm::vec3& base) {
	const float h = 0.5f;  // half an edge, so the whole edge measures 1
	Mesh mesh;
	mesh.reserve(36);

	quad(mesh, {-h, -h,  h}, { h, -h,  h}, { h,  h,  h}, {-h,  h,  h}, side);  // front  (+Z)
	quad(mesh, { h, -h, -h}, {-h, -h, -h}, {-h,  h, -h}, { h,  h, -h}, side);  // back   (-Z)
	quad(mesh, { h, -h,  h}, { h, -h, -h}, { h,  h, -h}, { h,  h,  h}, side);  // right  (+X)
	quad(mesh, {-h, -h, -h}, {-h, -h,  h}, {-h,  h,  h}, {-h,  h, -h}, side);  // left   (-X)
	quad(mesh, {-h,  h,  h}, { h,  h,  h}, { h,  h, -h}, {-h,  h, -h}, top );  // top    (+Y)
	quad(mesh, {-h, -h, -h}, { h, -h, -h}, { h, -h,  h}, {-h, -h,  h}, base);  // bottom (-Y)

	return mesh;
}


Mesh revolve(const std::vector<ProfilePoint>& profile, int segments) {
	// A whole turn closes the solid on itself, so there is no cut and no colour for one.
	return revolve(profile, segments, kTau, glm::vec3(0.0f));
}


Mesh revolve(const std::vector<ProfilePoint>& profile, int segments,
             float sweep, const glm::vec3& cut) {
	Mesh mesh;
	if (profile.size() < 2 || segments < 3) return mesh;

	// Where a profile point lands once it has been swung `angle` radians around the
	// Y axis: `radius` measures outwards from the axis, `height` along it.
	auto point_at = [](const ProfilePoint& point, float angle) {
		return glm::vec3(point.radius * std::cos(angle), point.height, point.radius * std::sin(angle));
	};
	auto on_ring = [&point_at](const ProfilePoint& point, float angle) {
		return Vertex{ point_at(point, angle), point.color };
	};

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
