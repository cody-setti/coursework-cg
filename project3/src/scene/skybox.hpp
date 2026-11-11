#pragma once
// --- SKY BOX ---
// Uploading the sky cube to the GPU and drawing it.
#include <glad/gl.h>                      // GLAD: OpenGL function pointers (e.g.: `glBufferData()`).

#include "loading/texture.hpp"            // `my::loadCubemap()`.


namespace my {

struct Skybox {
	GLuint     VAO = 0;  // Defaults to 0 = "absence of VAO", which marks a skybox that failed to load.
	GLuint     VBO = 0;
	GLuint texture = 0;
};

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

// Loads the 6 sky images into a cube-map and uploads the cube's corners to the GPU.
// `facePaths` follows `my::loadCubemap()`'s order. Returns a skybox with `VAO == 0` on failure.
inline my::Skybox loadSkybox(const char* const facePaths[6]) {
	my::Skybox skybox;
	skybox.texture = my::loadCubemap(facePaths);
	if ( skybox.texture == 0 ) {
		return {};
	}

	glGenVertexArrays(1, &skybox.VAO);
	glGenBuffers     (1, &skybox.VBO);

	glBindVertexArray(skybox.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, skybox.VBO);
	
	glBufferData(GL_ARRAY_BUFFER, sizeof(my::SKYBOX_VERTEX_DATA), my::SKYBOX_VERTEX_DATA, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return skybox;
}

// Draws the sky cube with its cube-map.
// OBS.: The skybox shader program and its uniforms (V and P) must be set before calling this.
inline void drawSkybox(const my::Skybox& skybox) {
	glBindVertexArray(skybox.VAO);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.texture);
	glDrawArrays (GL_TRIANGLES, 0, 36);// = 6 faces × 2 triangles × 3 corners.
}

// Frees the skybox's GPU memory (VAO, VBO and cube-map).
inline void deleteSkybox(my::Skybox& skybox) {
	glDeleteVertexArrays(1, &skybox.VAO);
	glDeleteBuffers     (1, &skybox.VBO);
	glDeleteTextures    (1, &skybox.texture);
}

}  // namespace my
