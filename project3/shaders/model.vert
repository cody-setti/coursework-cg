// --- MODEL VERTEX SHADER ---
// Runs once per vertex: decides WHERE the vertex lands on screen.

#version 460 core

layout (        location = 0) in      vec3   attributePos                 ;  // Spatial coordinates, read from attribute slot 0.
layout (        location = 1) in      vec2   attributeST                  ;  // Texture coordinates, read from attribute slot 1.
layout (        location = 2) in      vec3   attributeNrml                ;  // Surface direction  , read from attribute slot 2.

layout (std140, binding  = 0) uniform Camera { mat4 V; mat4 P; vec3 eye; };
layout (        location = 0) uniform mat4          M                     ;

                              out     vec3   varyingPos                   ;  // World space.
                              out     vec2   varyingST                    ;
                              out     vec3   varyingNrml                  ;  // World space.

void main() {
	const vec4 worldPos = M * vec4(attributePos, 1.0);
	const vec4  viewPos = V * worldPos               ;
	gl_Position         = P * viewPos                ;
	
	// Forward position, texture and normal coordinates to be
	// processed in the fragment shader for lighting.
	varyingPos          = worldPos.xyz           ;
	varyingST           = attributeST            ;
	varyingNrml         = mat3(M) * attributeNrml;
}
