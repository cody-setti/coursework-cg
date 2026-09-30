#pragma once  // This file may be reached by multiple others in the project.
              // `#pragma once` protects it from being read repeatedly in the 
              // compilation of any such one of these, which can cause errors!

// --- INCLUSIONS ---
#include <glm/glm.hpp>  // GLM = OpenGL Mathematics.
                        // Here we only need its `mat4` type; every matrix below is
                        // filled in by hand, for pedagogical purposes.

#include <cmath>        // For `std::cos` and `std::sin`.


// --- THE PRIMARY GEOMETRIC TRANSFORMATIONS ---
// REQUISITO 3:
//	"CADA OBJETO DEVE TER SUA PRÓPRIA MATRIZ DE TRANSFORMAÇÃO
//	COMPOSTA PELAS TRANSFORMAÇÕES GEOMÉTRICAS PRIMÁRIAS."
//
// In this file, we define the 5 primary geometric transformations:
//	1. Translation
//	2. Scaling
//	3. Rotation (X axis)
//	4. Rotation (Y axis)
//	5. Rotation (Z axis)
// From which the objects' composite transformation matrices are defined.
//
// THAT IS IT. This file has one job: defining our transformation alphabet.
// The definitive transformation matrices, sensitive to key presses and all,
// are defined in a later file!


// AUXILIARY CONSTANT
// The rotation transformations take RADIAN angles as arguments.
// τ is the length of a full turn in radians. That is, τ = 2π.
inline constexpr float kTau = 6.28318530717958647692f;
// NOTE: `inline` and `constexpr` are labels stuck to the declaration
//	`inline` means only a single copy of `kTau` is created, across all files!
//	`constexpr` (constant expression) means the value of `kTau` is simple enough
//	to be known at compile time! Yippee!

// A WORD ON COLUMN-MAJOR (CONSTRUCTING) ORDER
// Confusingly, although OpenGL treats points as column vectors at the mathematical
// level, at a computational one, when reading the 16 numbers that compose a 4x4 matrix,
// OpenGL (and consequently GLM) assumes they are passed COLUMN by COLUMN:
//	     a11, a21, a31, a41,
//	then a12, a22, a32, a42,
//	and so on and so forth.
// Therefore, the transformation definitions will LOOK transposed, but they are NOT!
// This issue is confined to this one file; no other file in the project ever has to
// think about it.

// 1. TRANSLATION
inline glm::mat4 translation(float tx, float ty, float tz) {
	return glm::mat4(
		1.0f, 0.0f, 0.0f, 0.0f,  // column 0
		0.0f, 1.0f, 0.0f, 0.0f,  // column 1
		0.0f, 0.0f, 1.0f, 0.0f,  // column 2
		  tx,   ty,   tz, 1.0f   // column 3
	);
}
// OBS.: For a header-only file, the `inline` label is CRUCIAL.
//       It's what permits us to define a function BODY in the header.
//       It prevents `translation` (and all other primary transforms)
//       from receiving multiple bodies, each time a file in the project
//       includes this header, which would result in an error!

// 2. SCALING
inline glm::mat4 scaling(float sx, float sy, float sz) {
	return glm::mat4(
		  sx, 0.0f, 0.0f, 0.0f,
		0.0f,   sy, 0.0f, 0.0f,
		0.0f, 0.0f,   sz, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}

// 3. ROTATION (X AXIS)
inline glm::mat4 rotation_x(float radians) {
	const float c = std::cos(radians);
	const float s = std::sin(radians);
	return glm::mat4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f,    c,    s, 0.0f,
		0.0f,   -s,    c, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}

// 4. ROTATION (Y AXIS)
inline glm::mat4 rotation_y(float radians) {
	const float c = std::cos(radians);
	const float s = std::sin(radians);
	return glm::mat4(
		   c, 0.0f,   -s, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		   s, 0.0f,    c, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}

// 5. ROTATION (Z AXIS)
inline glm::mat4 rotation_z(float radians) {
	const float c = std::cos(radians);
	const float s = std::sin(radians);
	return glm::mat4(
		   c,    s, 0.0f, 0.0f,
		  -s,    c, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}
