// Runs once per fragment: decides WHAT COLOR the fragment is.
#version 460 core

                     in      vec2      varyingST;
layout (binding = 0) uniform sampler2D tex      ;  // Reads from texture unit 0.
                     out     vec4      FragColor;

void main() {
	FragColor = texture(tex, varyingST);  // Color of the texture at this fragment's (s, t).

	// ALPHA CUTOUT: drop mostly transparent fragments so they write neither color nor depth.
	// Specifically, we use this for the pine model's leaves.
	if ( FragColor.a < 0.67 ) {
		discard;
	}
}
