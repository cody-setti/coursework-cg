// Runs once per vertex: decides WHERE the vertex lands on screen.
#version 460 core

layout (location = 0) in      vec3 attributePos;  // Spatial coordinates, read from attribute slot 0.
layout (location = 1) in      vec2 attributeST ;  // Texture coordinates, read from attribute slot 1.

layout (location = 0) uniform mat4          M  ;  // Uniform slot 0 does not clash with Attribute slot 0.
layout (location = 1) uniform mat4          V  ;
layout (location = 2) uniform mat4          P  ;

                      out     vec2   varyingST ;

void main() {
	gl_Position = P * V * M * vec4(attributePos, 1.0);
	varyingST   = attributeST;
}
