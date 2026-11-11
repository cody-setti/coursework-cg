#pragma once
// --- TEXTURE HELPERS ---
// Loading image files into OpenGL textures (2D and cube-map).
#include <iostream>                       // Input/Output Stream: output error messages using `std::cerr`.

#include <glad/gl.h>                      // GLAD: OpenGL function pointers (e.g.: `glTexImage2D()`).

#define STB_IMAGE_IMPLEMENTATION          // Makes the header below also emit the function bodies.
#include <stb_image.h>                    // Sean T. Barrett Image: decodes image files (.png, .jpg, ...) into raw pixel bytes.


namespace my {

// Loads an image file into a new OpenGL texture and returns its ID. Returns 0 on failure.
inline GLuint loadTexture(const char* path) {
	// Images are stored top row first, but OpenGL expects the bottom row first (t = 0 is the bottom).
	//   0             1              t                 
	// 0 ┌>───>───>───>┐  s         1 ┌─────────────┐   
	//   v     img.    │              ^   texture   │   
	//   │     file    │              │     map     │   
	//   v             │              ^             │   
	// 1 └─────────────┘            0 └>───>───>───>┘  s
	//   t                            0             1   
	stbi_set_flip_vertically_on_load(true);
	
	// DECODE IMAGE FILE
	int width, height, channels;
	unsigned char* pixels = stbi_load(
		path     ,
		            // A C++ function can return only one value. However, `stbi_load()` needs to give us back
		&width   ,  // 4 things: the (1) pixels, (2) width, (3) height and (4) channel count. The first is
		&height  ,  // returned normally. For the other three, it writes into their addresses. That's why we
		&channels,  // pass such addresses as arguments.
		
		4           // PARAM.: Channel count we WANT back.
		            // ARG.  : Always RGBA, so RGB and RGBA files can be uploaded the same way.
	);
	if ( pixels == nullptr ) {
		std::cerr << "Could not load image: " << path << " (" << stbi_failure_reason() << ")" << std::endl;
		return 0;  // Texture ID 0 is reserved for "absence of texture".
	}
	
	// CREATE TEXTURE AND UPLOAD TO GPU
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	
	glTexImage2D(
		GL_TEXTURE_2D   ,  // PARAM.: Target texture slot.
		0               ,  // PARAM.: Mipmap level.
		                   // ARG.  : 0, the full-size image (smaller levels are generated below).
		GL_RGBA8        ,  // PARAM.: Format the GPU stores the texture in.
		width, height   ,
		0               ,  // PARAM.: Border. Must be 0 (legacy parameter).
		GL_RGBA         ,  // PARAM.: Channel order of `pixels`.
		GL_UNSIGNED_BYTE,  // PARAM.: Type of each channel in `pixels` (0-255).
		pixels             // PARAM.: Pointer to the data to copy (CPU -> GPU).
	);
	stbi_image_free(pixels);  // The GPU has its own copy now.
	
	// CONFIGURE CLOSE/FAR TEXTURE SAMPLING
	// TEXEL > PIXEL
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	// TEXEL < PIXEL
	// Mipmaps (Multum In Parvo Maps):
	// pre-shrunk copies of the texture, used when it is drawn far away (avoids shimmering and moiré).
	//
	// e.g.:
	// level  0: 2048 × 1024  (the original)
	// level  1: 1024 ×  512
	// level  2:  512 ×  256
	//     ...
	// level 11:    1 ×    1
	//
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	// CONFIGURE WRAPPING (what to sample when a texture coordinate falls outside [0, 1])
	// `GL_REPEAT` ignores the integer part (e.g.: s = 2.3 samples the same spot as s = 0.3), tiling the texture.
	// OBS.: OpenGL names the normalized texture axes S and T.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);  // Horizontal axis.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  // Vertical   axis.
	
	glBindTexture(GL_TEXTURE_2D, 0);
	return texture;
}

// Loads 6 image files into a new OpenGL cube-map texture and returns its ID. Returns 0 on failure.
// `facePaths` must follow OpenGL's face order: +X (right), -X (left), +Y (top), -Y (bottom), +Z (front), -Z (back).
inline GLuint loadCubemap(const char* const facePaths[6]) {
	// Unlike 2D textures, cube-map faces are expected top row first, just like image files: no flip needed.
	// OBS.: This setting is global, but `my::loadTexture()` sets it back to `true` every time it is called.
	stbi_set_flip_vertically_on_load(false);

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

	// DECODE EACH FACE AND UPLOAD IT TO GPU
	for ( int i = 0; i < 6; i++ ) {
		int width, height, channels;
		unsigned char* pixels = stbi_load(facePaths[i], &width, &height, &channels, 4);
		if ( pixels == nullptr ) {
			std::cerr << "Could not load image: " << facePaths[i] << " (" << stbi_failure_reason() << ")" << std::endl;
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
			glDeleteTextures(1, &texture);
			return 0;
		}
	
		// OBS.:
		//       GL_TEXTURE_CUBE_MAP_POSITIVE_X + 0 <--> +X
		//       GL_TEXTURE_CUBE_MAP_POSITIVE_X + 1 <--> -X
		//       GL_TEXTURE_CUBE_MAP_POSITIVE_X + 2 <--> +Y
		//         ...                                     
		//       GL_TEXTURE_CUBE_MAP_POSITIVE_X + 5 <--> -Z
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		stbi_image_free(pixels);
	}

	// CONFIGURE SAMPLING
	// The sky is always drawn at roughly the same size on screen: no mipmaps needed.
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// HIDE THE SEAMS BETWEEN FACES
	// `GL_LINEAR` blends each sample with its neighbor texels. On a face's border, `GL_REPEAT` would pick that neighbor
	// from the OPPOSITE border of the SAME face, drawing a visible line along every cube edge.
	//
	// `GL_TEXTURE_CUBE_MAP_SEAMLESS` blends with the ADJACENT face's border texels.
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	// OBS.: It's a global switch (affects every cube-map), not a per-texture parameter.

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	return texture;
}

}  // namespace my
