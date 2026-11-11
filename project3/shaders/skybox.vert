// Runs once per vertex of the sky cube: decides WHERE the vertex lands on screen.
#version 460 core

layout (location = 0) in      vec3 aCorner;

// Same uniform slots as "textured.vert", so `main.cpp` can reuse its constants.
// OBS.: NO MODEL MATRIX: THE SKY IS NEVER MOVED, ROTATED NOR SCALED.
layout (location = 1) uniform mat4  V     ;
layout (location = 2) uniform mat4  P     ;

                      out     vec3  vDir  ;

void main() {
	// Removing V's translation (its 4th column) keeps only the camera's rotation: the cube stays centered
	// on the camera wherever it moves, so the sky looks "infinitely" far away.
	mat4 skyV = mat4(mat3(V));

	vec4 clipPos = P * skyV * vec4(aCorner, 1.0);

	// After the perspective division, depth = z / w. Setting z = w makes it 1.0, the farthest depth possible,
	// so EVERY OTHER MODEL IS DRAWN IN FRONT OF THE SKY CUBE.
	gl_Position = clipPos.xyww;
	
	// A cube-map is sampled by a DIRECTION from the cube's center. Since the cube is centered on the origin,
	// each corner's position is numerically identical to its direction.
	vDir = aCorner;
}
