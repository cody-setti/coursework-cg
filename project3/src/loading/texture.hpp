// --- TEXTURE ---
// Loads image files into OpenGL textures (2D and cube-map).

#pragma once                              // `#include` literally pastes a file's text in place.
                                          // `#pragma once` protects `texture.hpp` from being double pasted.

#include <cmath>                          // C Mathematics: `std::log2()`, `std::floor()`.
#include <algorithm>                      // `std::max()`.

#include <iostream>                       // Input/Output Stream: output error messages using `std::cerr`.

#include <glad/gl.h>                      // GLAD: OpenGL function pointers (e.g.: `glTextureStorage2D()`).

#include <stb_image.h>                    // Sean T. Barrett Image: decodes image files (.png, .jpg, ...) into raw pixel bytes.


namespace tex {

// FUNCTIONS
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
	unsigned char* const pixels = stbi_load(
		path     ,
		            // A C++ function can return only one value. However, `stbi_load()` needs to give us back
		&width   ,  // 4 things: the (1) pixels, (2) width, (3) height and (4) channel count. The first is
		&height  ,  // returned normally. For the other three, it writes into their addresses. That's why we
		&channels,  // pass such addresses as arguments.
		
		4           // PARAM.: Channel count we WANT back.
		            // ARG.  : Always RGBA, so RGB and RGBA files can be uploaded the same way.
	);
	if ( pixels == nullptr ) {
		std::cerr << "Could not decode image: " << path << " (" << stbi_failure_reason() << ")" << std::endl;
		return 0;  // Texture ID 0 is reserved for "absence of texture".
	}
	
	// CREATE TEXTURE AND UPLOAD TO GPU
	// Sampling textures is not trivial, as pixels and texels rarely align.
	//
	// When the texels are larger than the pixels, a pixel intersects at most 4 texels.
	// We can simply interpolate them to determine its color.
	//
	// However, when they are smaller, a single pixel can cover a very large number of texels.
	// Which one do we choose to color it?
	//
	// The solution is a mipmap (Multum in Parvo Map):
	// Pre-shrunk copies of the texture, used when it is drawn far away (avoids shimmering and moiré).
	//
	// e.g.:
	// level  0: 2048 × 1024  (the original)
	// level  1: 1024 ×  512                
	// level  2:  512 ×  256                
	//      …                               
	// level 10:    2 ×    1                
	// level 11:    1 ×    1                
	//
	// Therefore, the first step in uploading a texture is figuring out how many levels its mipmap requires.
	const GLsizei levels = 1 + (GLsizei)std::floor( std::log2( std::max(width, height) ) );
	
	GLuint texture;
	glCreateTextures(
		GL_TEXTURE_2D,  // PARAM.: The kind of texture to create.
		1            ,  // PARAM.: How many textures to create.
		&texture        // PARAM.: Where to write their IDs (an array of GLuints).
	);
	
	// Reserve GPU memory for every level at once. The storage is IMMUTABLE.
	glTextureStorage2D(
		texture      ,
		levels       ,  // PARAM.: Number of mipmap levels to reserve (including the full size image).
		GL_RGBA8     ,  // ARG.  : 4 channels (R, G, B, A) of 8 bits each, normalized: byte 255 reaches the shader as 1.0.
		width, height   // PARAM.: Size of level 0, in texels.
	);
	
	// "Sub" image: writes a rectangle of pixels into storage that ALREADY exists. It never allocates.
	// OBS.: `GL_RGBA8` above described the GPU's copy, while `GL_RGBA` + `GL_UNSIGNED_BYTE` below describe OURS (CPU).
	//       They are separate because they may differ (e.g.: an RGB file into RGBA storage): the driver converts between them.
	glTextureSubImage2D(
		texture         ,
		0               ,  // PARAM.: Mipmap level to write.
		0               ,  // PARAM.: x offset of the rectangle (its left   edge), in texels.
		0               ,  // PARAM.: y offset of the rectangle (its bottom edge), in texels.
		width, height   ,  // PARAM.: Size of the rectangle, in texels.
		GL_RGBA         ,  // PARAM.: Channel order of `pixels`.
		GL_UNSIGNED_BYTE,  // PARAM.: Type of each channel in `pixels`.
		pixels             // PARAM.: Pointer to the data to copy (CPU -> GPU).
	);
	
	stbi_image_free(pixels);  // The GPU has its own copy now.
	
	// CONFIGURE CLOSE/FAR TEXTURE SAMPLING
	glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR)                  ;  // Magnification
	glGenerateTextureMipmap(texture)                                                ;  // Minification
	glTextureParameteri    (texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	
	// CONFIGURE WRAPPING (what to sample when a texture coordinate falls outside [0, 1])
	// `GL_REPEAT` ignores the integer part (e.g.: s = 2.3 samples the same spot as s = 0.3), tiling the texture.
	// OBS.: OpenGL names the normalized texture axes S and T.
	glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_REPEAT);  // Horizontal axis.
	glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_REPEAT);  // Vertical   axis.
	
	return texture;
}

// Loads 6 image files into a new OpenGL cube-map texture and returns its ID. Returns 0 on failure.
// `facePaths` must follow OpenGL's face order: +X (right), -X (left), +Y (top), -Y (bottom), +Z (front), -Z (back).
inline GLuint loadCubemap(const char* const facePaths[6]) {
	// Unlike 2D textures, cube-map faces are expected top row first, just like image files: no flip needed.
	// OBS.: This setting is global, but `tex::loadTexture()` sets it back to `true` every time it is called.
	stbi_set_flip_vertically_on_load(false);
	
	GLuint texture;
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &texture);
	
	// DECODE EACH FACE AND UPLOAD IT TO GPU
	for ( int i = 0; i < 6; i++ ) {
		int width, height, channels;
		unsigned char* const pixels = stbi_load(facePaths[i], &width, &height, &channels, 4);
		if ( pixels == nullptr ) {
			std::cerr << "Could not decode image: " << facePaths[i] << " (" << stbi_failure_reason() << ")" << std::endl;
			glDeleteTextures(1, &texture);
			return 0;
		}
		
		if ( i == 0 ) glTextureStorage2D(texture, 1, GL_RGBA8, width, height);
		glTextureSubImage3D(texture, 0, 0, 0, i, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		stbi_image_free(pixels);
	}
	
	// CONFIGURE SAMPLING
	// The sky is always drawn at roughly the same size on screen: no mipmaps needed.
	glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
	return texture;
}

}  // namespace tex
