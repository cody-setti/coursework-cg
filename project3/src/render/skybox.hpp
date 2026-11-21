// --- SKY BOX ---
// Loads the sky cube to the GPU, draws it and frees it.

#pragma once                              // `#include` literally pastes a file's text in place.
                                          // `#pragma once` protects `skybox.hpp` from being double pasted.

#include <glad/gl.h>                      // GLAD: OpenGL function pointers (e.g.: `glNamedBufferStorage()`).

#include "loading/texture.hpp"            // `tex::loadCubemap()`.


namespace sky {

// TYPES
struct Skybox {
	GLuint     VAO = 0;  // Defaults to 0 = "absence of VAO", which marks a skybox that failed to load.
	GLuint     VBO = 0;
	GLuint texture = 0;
};


// CONSTANTS
constexpr float SKYBOX_VERTEX_DATA[] = {
	// +X (right)
	 1.0f, -1.0f, -1.0f,   1.0f, -1.0f,  1.0f,   1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,   1.0f,  1.0f, -1.0f,   1.0f, -1.0f, -1.0f,
	// -X (left)
	-1.0f, -1.0f,  1.0f,  -1.0f, -1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,
	// +Y (top)
	-1.0f,  1.0f, -1.0f,   1.0f,  1.0f, -1.0f,   1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f,  1.0f, -1.0f,
	// -Y (bottom)
	-1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,   1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,   1.0f, -1.0f,  1.0f,
	// +Z (front)
	-1.0f, -1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,   1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,   1.0f, -1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,
	// -Z (back)
	-1.0f,  1.0f, -1.0f,  -1.0f, -1.0f, -1.0f,   1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,   1.0f,  1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,
};


// FUNCTIONS
// Loads the 6 sky images into a cube-map and uploads the cube's corners to the GPU.
// `facePaths` follows `tex::loadCubemap()`'s order. Returns a skybox with `VAO == 0` on failure.
inline sky::Skybox loadSkybox(const char* const facePaths[6]) {
	sky::Skybox skybox;
	skybox.texture = tex::loadCubemap(facePaths);
	if ( skybox.texture == 0 ) {
		return {};
	}
	
	glCreateVertexArrays(1, &skybox.VAO);
	glCreateBuffers     (1, &skybox.VBO);
	
	glNamedBufferStorage(skybox.VBO, sizeof(sky::SKYBOX_VERTEX_DATA), sky::SKYBOX_VERTEX_DATA, 0);
	
	glVertexArrayVertexBuffer (skybox.VAO, 0, skybox.VBO, 0, 3 * sizeof(float));  // Attach the VAO to the VBO and set the vertex stride.
	glVertexArrayAttribFormat (skybox.VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);          // Format the first (and only) attribute: cube corners.
	glVertexArrayAttribBinding(skybox.VAO, 0, 0);                                 // Bind the attribute to the buffer SLOT it reads from.
	glEnableVertexArrayAttrib (skybox.VAO, 0);                                    // Enable reading the attribute data.
	
	return skybox;
}

// Draws the sky cube with its cube-map.
// OBS.: Before this draw function below runs, the sky box shader program must be in use, and the
//       camera buffer must be filled and bound to binding point 0 (peep `main.cpp`).
inline void drawSkybox(const sky::Skybox& skybox) {
	// HIDE THE SEAMS BETWEEN FACES
	// `GL_LINEAR` blends each sample with its neighbor texels. On a face's border, `GL_REPEAT` would pick that neighbor
	// from the OPPOSITE border of the SAME face, drawing a visible line along every cube edge.
	//
	// `GL_TEXTURE_CUBE_MAP_SEAMLESS` blends with the ADJACENT face's border texels.
	// OBS.: It's a global switch (affects every cube-map), not a per-texture parameter:
	//       disabled again after drawing, so it doesn't leak past the sky.
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	
	glBindVertexArray(skybox.VAO);
	glBindTextureUnit(0, skybox.texture);
	glDrawArrays (GL_TRIANGLES, 0, 36);
	//                              = 6 faces × 2 triangles × 3 corners.
	
	glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

// Frees the skybox's GPU memory (VAO, VBO and cube-map).
inline void deleteSkybox(sky::Skybox& skybox) {
	glDeleteVertexArrays(1, &skybox.VAO);
	glDeleteBuffers     (1, &skybox.VBO);
	glDeleteTextures    (1, &skybox.texture);
}

}  // namespace sky
