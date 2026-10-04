// Runs once per fragment of the sky cube: decides WHAT COLOR the fragment is.
#version 460 core

                     in      vec3        vDir      ;
layout (binding = 0) uniform samplerCube sky       ;  // Reads from texture unit 0.
                     out     vec4        FragColor ;

void main() {
	// The direction picks a face of the cube, then a texel on that face.
	//
	// > Its largest component (in magnitude) says which face to pick from.
	// > The other two, normalized by it, give the (u,v) coordinates.
	//         ┌──────┐              
	//         │ top  │              
	//         │ +Y   │              
	//  ┌──────┼──────┼──────┬──────┐
	//  │ left │front │right │ back │
	//  │ -X   │ +Z   │ +X   │  -Z  │
	//  └──────┼──────┼──────┴──────┘
	//         │bottom│              
	//         │ -Y   │              
	//         └──────┘              
	FragColor = texture(sky, vDir);
}
