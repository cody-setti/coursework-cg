#pragma once  // Ask the compiler to read this file only once per translation unit,
              // no matter how many other headers reach for it.

// --- INCLUSIONS ---
#include <glm/glm.hpp>  // GLM = OpenGL Mathematics.
                        // Here we only need its `mat4` type; every matrix below is
                        // filled in by hand, on purpose (see the note further down).

#include <cmath>        // For `std::cos` and `std::sin`.


// --- THE PRIMARY GEOMETRIC TRANSFORMATIONS ---
// Every object in the scene carries ONE transformation matrix, and that matrix is
// always a product of the five functions below. They are the alphabet; the objects
// in `scene.cpp` are the sentences.
//
// A WORD ON COLUMN-MAJOR ORDER, SAID ONCE SO IT NEVER HAS TO BE SAID AGAIN:
// `glm::mat4`'s 16-argument constructor fills the matrix COLUMN by column, not row
// by row. So a literal written out in source order looks like the TRANSPOSE of the
// matrix drawn in a textbook. That is why, in every function below, the drawing in
// the comment and the numbers in the code appear flipped across the diagonal. It is
// not a bug, and confining it to this one file is precisely the point: no other file
// in the project ever has to think about it.
//
// Reading a composition such as `translation(...) * rotation_z(...) * scaling(...)`
// goes RIGHT TO LEFT: the vertex is scaled first, then rotated, then moved.

constexpr float kPi = 3.14159265358979323846f;


// Slide every point by (tx, ty, tz).
//
//     | 1  0  0  tx |
//     | 0  1  0  ty |
//     | 0  0  1  tz |
//     | 0  0  0   1 |
//
// This is the transformation that needs the fourth dimension: with 3x3 matrices a
// translation is simply not expressible, because a matrix always maps the origin to
// itself. The `w = 1` we tack onto each vertex in the shader is what gives the `t`
// column something to multiply.
inline glm::mat4 translation(float tx, float ty, float tz) {
	return glm::mat4(
		1.0f, 0.0f, 0.0f, 0.0f,  // column 0
		0.0f, 1.0f, 0.0f, 0.0f,  // column 1
		0.0f, 0.0f, 1.0f, 0.0f,  // column 2
		  tx,   ty,   tz, 1.0f   // column 3 <- the translation lives here
	);
}


// Stretch every point away from the origin by (sx, sy, sz).
//
//     | sx  0   0  0 |
//     |  0 sy   0  0 |
//     |  0  0  sz  0 |
//     |  0  0   0  1 |
//
// Note "away from the ORIGIN": scaling an object that was modelled far from (0,0,0)
// moves it as well as resizing it. The cure is the same three-step recipe used for
// off-centre rotations -- bring the object to the origin, scale, put it back.
inline glm::mat4 scaling(float sx, float sy, float sz) {
	return glm::mat4(
		  sx, 0.0f, 0.0f, 0.0f,
		0.0f,   sy, 0.0f, 0.0f,
		0.0f, 0.0f,   sz, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}


// Spin every point around the X axis. Y tips towards Z.
//
//     | 1    0       0     0 |
//     | 0  cos(a)  -sin(a) 0 |
//     | 0  sin(a)   cos(a) 0 |
//     | 0    0       0     1 |
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


// Spin every point around the Y axis. Z tips towards X.
//
//     |  cos(a)  0  sin(a)  0 |
//     |    0     1    0     0 |
//     | -sin(a)  0  cos(a)  0 |
//     |    0     0    0     1 |
//
// The odd one out: its minus sign sits in the bottom-left rather than the top-right,
// because rotating X->Z runs against the cyclic order X->Y->Z->X the other two follow.
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


// Spin every point around the Z axis. X tips towards Y.
// With the scene facing us down the Z axis, this is the one that looks like plain
// rotation on the screen.
//
//     | cos(a)  -sin(a)  0  0 |
//     | sin(a)   cos(a)  0  0 |
//     |   0        0     1  0 |
//     |   0        0     0  1 |
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
