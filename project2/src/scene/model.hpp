#pragma once
// --- MODEL ---
// Uploading textured .obj models to the GPU and drawing them.
#include <vector>                         // The `std::vector` object is used to store material ranges.
#include <string>                         // The `std::string` object is used for material names.
#include <map>                            // The `std::map` object is used to look up textures by material name.

#include <glad/gl.h>                      // GLAD: OpenGL function pointers (e.g.: `glBufferData()`).

#include "loading/wavefront.hpp"          // `my::parseOBJ()`, `my::parseMTL()`, `my::MaterialRange` and `my::FLOATS_PER_VERTEX`.
#include "loading/texture.hpp"            // `my::loadTexture()`.


namespace my {

// A model ready to be drawn: its vertex data lives in GPU memory, split into material ranges.
struct Model {
	GLuint                         VAO = 0 ;  // Defaults to 0 = "absence of VAO", which marks a model that failed to load.
	GLuint                         VBO = 0 ;
	std::vector<my::MaterialRange> ranges  ;
	std::map<std::string, GLuint>  textures;
};

// Loads a Wavefront .obj file (and its .mtl textures) and uploads its vertex data to the GPU.
// Returns a model with `VAO == 0` on failure.
inline my::Model loadModel(const char* objPath) {
	my::ParsedOBJ obj = my::parseOBJ(objPath);
	if ( obj.vertexData.empty() ) {
		return {};
	}
	
	my::Model model;
	model.ranges = obj.ranges;
	
	
	// Upload material images to texture objects on the GPU and save their addresses
	// in `model`s map `model.textures`.
	std::map<std::string, std::string> imagePaths = my::parseMTL( obj.mtlPath.c_str() );
	
	for ( const auto& [material, path] : imagePaths ) {
		GLuint texture = my::loadTexture( path.c_str() );
		if ( texture != 0 ) {
			model.textures[material] = texture;
		}
	}
	if ( model.textures.empty() ) {
		return {};
	}
	
	
	// OBS.:
	// > VBO (Vertex Buffer Object): holds the vertex data in GPU memory.
	// > VAO (Vertex Array  Object): describes HOW such data is laid out.
	//
	// IDs for VAO and VBO used in the CPU are stored in `model.VAO` and `model.VBO`.
	
	// Populate `model.VAO` and `model.VBO` with available driver object IDs, that is:
	// a number not already reserved to refer to an existing driver object.
	glGenVertexArrays(1, &model.VAO);
	glGenBuffers     (1, &model.VBO);
	
	// BIND the objects of IDs `model.VAO` and `model.VBO` to the OpenGL context.
	// Since these are their first binds, the `glBind*()` function calls also make sure to create such objects.
	glBindVertexArray(model.VAO);
	glBindBuffer(
		GL_ARRAY_BUFFER,   // PARAM.: Bind target.
		                   // ARG.  : VERTEX DATA slot (a slot is a field in the driver's context state).
		
		model.VBO          // PARAM.: ID of the buffer to bind.
	);
	
	// Upload vertex data from the `my::ParsedOBJ` struct to the VBO.
	glBufferData(
		GL_ARRAY_BUFFER                      ,  // PARAM.: Target buffer slot.
		                                        // ARG.  : VERTEX DATA slot.
		
		obj.vertexData.size() * sizeof(float),  // PARAM.: Size of data, in bytes.
		
		obj.vertexData.data()                ,  // PARAM.: Pointer to the data to copy (CPU -> GPU).
		                                        // ARG.  : Pointer to VERTEX DATA.
		
		GL_STATIC_DRAW                          // PARAM.: Usage hint.
		                                        // ARG.  : Data is set once and drawn many times.
	);
	
	// Writes into the currently bound VAO.
	// OBS.: Saves in the VAO a reference to the VBO (the buffer currently bound to slot `GL_ARRAY_BUFFER`).
	glVertexAttribPointer(
		0                                    ,  // PARAM.: Attribute location.
		                                        // ARG.  : Location 0, which stores positional info. (as determined in vertex shader source code).
		
		3                                    ,  // PARAM.: Number of attribute components per vertex.
		                                        // ARG.  : 3, corresponding to spatial coordinates x, y and z.
		
		GL_FLOAT                             ,  // PARAM.: Type of each component.
		
		GL_FALSE                             ,  // PARAM.: Boolean that answers "Normalize?"
		
		my::FLOATS_PER_VERTEX * sizeof(float),  // PARAM.: Stride (in bytes) from one vertex's data to the next.
		
		(void*)0                                // PARAM.: Byte offset from VBO's address to that of the first vertex
		                                        //         attribute in question.
		                                        //         It is passed as a memory pointer (`void*`) instead of an `int`
		                                        //         simply for legacy reasons: vertex data used to be stored in
		                                        //         CPU memory instead of the VBO.
		                                        // ARG.  : 0 offset. The VBO's very first element is already an instance of
		                                        //         the "position" attribute.
	);
	// Same as above, for the texture coordinates: 2 floats (s, t), stored right after each vertex's 3 position floats.
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, my::FLOATS_PER_VERTEX * sizeof(float), (void*)(3 * sizeof(float)));
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	// Unbind `model.VAO` and `model.VBO` from this OpenGL context (avoid accidental state changes).
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	return model;
}

// Draws every material range of `model`, each with its own texture.
// OBS.: The shader program and its uniforms (e.g.: the Model matrix) must be set before calling this.
inline void drawModel(my::Model& model) {
	glBindVertexArray(model.VAO);
	// Since the object `model.VAO` points to has a reference to the one `model.VBO` points to,
	// it is not necessary to re-bind the VBO to the `GL_ARRAY_BUFFER` slot.
	
	// Draw the model one material range at a time.
	for ( const my::MaterialRange& range : model.ranges ) {
		// Bound to texture unit 0, which the fragment shader's sampler reads from.
		glBindTexture(GL_TEXTURE_2D, model.textures[range.material]);
		// OBS.: If `range.material` has no texture, `[]` inserts it into the map with ID 0 ("absence of texture"),
		//       which samples as black. Deleting texture 0 at shutdown is silently ignored by OpenGL.
		
		glDrawArrays(
			GL_TRIANGLES, // Each trio of vertices defines a triangle.
			range.first , // Starting vertex.
			range.count   // How many vertices to process.
		);
	}
}

// Frees the model's GPU memory (VAO, VBO and textures).
inline void deleteModel(my::Model& model) {
	glDeleteVertexArrays(1, &model.VAO);
	glDeleteBuffers     (1, &model.VBO);
	for ( const auto& [material, texture] : model.textures ) {
		glDeleteTextures(1, &texture);
	}
}

}  // namespace my
