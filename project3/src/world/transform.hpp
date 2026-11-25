// --- TRANSFORM ---
// Placement (position, angle and scale) of a model instance in the world,
// and the Model matrix (M) it amounts to.

#pragma once                             // `#include` literally pastes a file's text in place.
                                         // `#pragma once` protects `transform.hpp` from being double pasted.

#include <glm/glm.hpp>                   // GL Mathematics: provides GLSL-compatible vector and matrix types.
#include <glm/ext/matrix_transform.hpp>  // e.g.: `translate()`, `rotate()`, `scale()`.


namespace xform {

// TYPES
struct Transform {
	glm::vec3 position{0.0f};
	glm::vec3   angles{0.0f};  // In radians, around the x, y and z axes.
	glm::vec3    scale{1.0f};
};


// FUNCTIONS
// M = T · Ry · Rx · Rz · S
// Read right to left: a vertex is scaled, turned around z, then x, then y, and only then moved to `position`.
inline glm::mat4 modelMatrix(const xform::Transform& transform) {
	glm::mat4 M = glm::mat4(1.0f);
	
	M = glm::translate(M, transform.position);
	M = glm::rotate   (M, transform.angles.y, glm::vec3(0.0f, 1.0f, 0.0f));
	M = glm::rotate   (M, transform.angles.x, glm::vec3(1.0f, 0.0f, 0.0f));
	M = glm::rotate   (M, transform.angles.z, glm::vec3(0.0f, 0.0f, 1.0f));
	M = glm::scale    (M, transform.scale);
	
	return M;
}

// M = T · C · S
// Like `modelMatrix()`, but the rotation C mirrors the camera's.
// OBS.: Therefore, the model's Euler angles are ignored state.
inline glm::mat4 billboardMatrix(const xform::Transform& transform, const glm::mat4& V) {
	glm::mat4 M = glm::mat4(1.0f);
	
	M = glm::translate(M, transform.position);
	
	// The inverse of a rotation matrix is its transpose.
	M = M * glm::mat4( glm::transpose(glm::mat3(V)) );
	
	M = glm::scale(M, transform.scale);
	
	return M;
}

}  // namespace xform
