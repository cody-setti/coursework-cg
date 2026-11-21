// --- MODEL ---
// Loads models (with texture and normals) in the GPU, draws them and frees them.

#pragma once                              // `#include` literally pastes a file's text in place.
                                          // `#pragma once` protects `model.hpp` from being double pasted.

#include <vector>                         // The `std::vector` object is used to store material ranges.
#include <string>                         // The `std::string` object is used for material names.
#include <map>                            // The `std::map` object is used to look up textures and reflectance values by material name.

#include <glad/gl.h>                      // GLAD: OpenGL function pointers (e.g.: `glNamedBufferStorage()`).
#include <glm/glm.hpp>                    // GL Mathematics: `glm::mat4`.

#include "loading/wavefront.hpp"          // `wavef::parseOBJ()`, `wavef::parseMTL()`, `wavef::MaterialRange`, `wavef::ParsedMaterial` and `wavef::FLOATS_PER_VERTEX`.
#include "loading/texture.hpp"            // `tex::loadTexture()`.


namespace mdl {

// TYPES
// A model ready to be drawn: its vertex data lives in GPU memory, split into material ranges.
struct Model {
	GLuint                                       VAO = 0  ;  // Defaults to 0 = "absence of VAO", which marks a model that failed to load.
	GLuint                                       VBO = 0  ;
	std::vector<wavef::MaterialRange>            ranges   ;
	std::map<std::string, wavef::ParsedMaterial> materials;
	std::map<std::string, GLuint>                textures ;
};


// CONSTANTS
// Uniform locations, as fixed by the `layout (location = ...)` qualifiers in "shaders/model.vert" and "shaders/model.frag".
// OBS.: Locations belong to the whole PROGRAM, not to one stage: the fragment shader's uniforms can't reuse the vertex shader's 0.
constexpr GLint MODEL_M_LOCATION           = 0;
constexpr GLint MODEL_KD_LOCATION          = 1;
constexpr GLint MODEL_KS_LOCATION          = 2;
constexpr GLint MODEL_NS_LOCATION          = 3;
constexpr GLint MODEL_AMBIENT_LOCATION     = 4;
constexpr GLint MODEL_LIGHT_POS_LOCATION   = 5;
constexpr GLint MODEL_LIGHT_COLOR_LOCATION = 6;


// FUNCTIONS
// Loads a Wavefront .obj file (and its .mtl textures) and uploads its vertex data to the GPU.
// Returns a model with `VAO == 0` on failure.
inline mdl::Model loadModel(const char* objPath) {
	const wavef::ParsedOBJ obj = wavef::parseOBJ(objPath);
	if ( obj.vertexData.empty() ) {
		return {};
	}
	
	mdl::Model model;
	model.ranges = obj.ranges;
	
	
	model.materials = wavef::parseMTL( obj.mtlPath.c_str() );
	
	// Upload material images to texture objects on the GPU and save their addresses in `model`'s texture map.
	for ( const auto& [name, material] : model.materials ) {
		const GLuint texture = tex::loadTexture( material.imagePath.c_str() );
		if ( texture != 0 ) {
			model.textures[name] = texture;
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
	
	// Create a VAO and a VBO and store their addresses in `model.VAO` and `model.VBO`, respectively.
	glCreateVertexArrays(1, &model.VAO);
	glCreateBuffers     (1, &model.VBO);
	
	// Upload vertex data from the `wavef::ParsedOBJ` struct to the VBO.
	glNamedBufferStorage(
		model.VBO                            ,
		obj.vertexData.size() * sizeof(float),  // PARAM.: Size of data, in bytes.
		obj.vertexData.data()                ,  // PARAM.: Pointer to the data to copy (CPU -> GPU).
		                                        // ARG.  : Pointer to VERTEX DATA.
		0                                       // PARAM.: Flags (what you are allowed to do with the buffer).
		                                        // ARG.  : No flags, therefore nothing. The content is final.
	);
	
	// Attach the VAO to the VBO and set the vertex stride.
	glVertexArrayVertexBuffer(
		model.VAO                               ,
		0                                       ,  // PARAM.: VAO binding index.
		model.VBO                               ,
		0                                       ,  // PARAM.: VBO offset.
		wavef::FLOATS_PER_VERTEX * sizeof(float)   // PARAM.: Vertex stride.
	);
	
	// Format the VAO according to how each vertex attribute is stored in the VBO.
	glVertexArrayAttribFormat (model.VAO, 0, 3, GL_FLOAT, GL_FALSE, 0                );  // Position.
	glVertexArrayAttribFormat (model.VAO, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float));  // Texture.
	glVertexArrayAttribFormat (model.VAO, 2, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float));  // Normal.
	
	// Bind an attribute to the BUFFER SLOT it reads from.
	glVertexArrayAttribBinding(model.VAO, 0, 0)                                       ;  // Position.
	glVertexArrayAttribBinding(model.VAO, 1, 0)                                       ;  // Texture.
	glVertexArrayAttribBinding(model.VAO, 2, 0)                                       ;  // Normal.
	// OBS.: All attributes read from the same binding index ~ 0 ~, as a single VBO was created.
	//       After all, we called `glVertexArrayVertexBuffer()` only once.
	
	// Enable reading the attributes from the VBO.
	glEnableVertexArrayAttrib (model.VAO, 0)                                          ;  // Position.
	glEnableVertexArrayAttrib (model.VAO, 1)                                          ;  // Texture.
	glEnableVertexArrayAttrib (model.VAO, 2)                                          ;  // Normal.
	
	return model;
}

// Draws every material range of `model`, each with its own texture, placed in the world by `M`.
// OBS.: Before this draw function below runs, the model shader program must be in use, and the
//       camera buffer must be filled and bound to binding point 0 (peep `main.cpp`).
inline void drawModel(const mdl::Model& model, const glm::mat4& M) {
	glUniformMatrix4fv(mdl::MODEL_M_LOCATION, 1, GL_FALSE, &M[0][0]);
	
	glBindVertexArray(model.VAO);
	
	// Draw the model one material range at a time.
	for ( const wavef::MaterialRange& range : model.ranges ) {
		
		const auto   foundTxtr = model.textures.find(range.mtlName)                             ;
		const GLuint texture   = ( foundTxtr != model.textures.end() ) ? (*foundTxtr).second : 0;
		
		// Bind to texture unit 0, which the fragment shader's sampler reads from.
		glBindTextureUnit(0, texture);
		// OBS.: ID 0, "absence of texture", samples as black.
		
		const auto                  foundMtl = model.materials.find(range.mtlName)                                                 ;
		const wavef::ParsedMaterial material = ( foundMtl != model.materials.end() ) ? (*foundMtl).second : wavef::ParsedMaterial{};
		
		glUniform3fv(mdl::MODEL_KD_LOCATION, 1, &material.Kd[0]);
		glUniform3fv(mdl::MODEL_KS_LOCATION, 1, &material.Ks[0]);
		glUniform1f (mdl::MODEL_NS_LOCATION,     material.Ns   );
		
		glDrawArrays(
			GL_TRIANGLES, // Each trio of vertices defines a triangle.
			range.first , // Starting vertex.
			range.count   // How many vertices to process.
		);
	}
}

// Frees the model's GPU memory (VAO, VBO and textures).
inline void deleteModel(mdl::Model& model) {
	glDeleteVertexArrays(1, &model.VAO);
	glDeleteBuffers     (1, &model.VBO);
	for ( const auto& [material, texture] : model.textures ) {
		glDeleteTextures(1, &texture);
	}
}

}  // namespace mdl
