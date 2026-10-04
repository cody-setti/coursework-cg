#pragma once
// --- SHADER HELPERS ---
// Reading, compiling and linking GLSL shader source code.
#include <string>                         // The `std::string` object is used to manage shader source code.
#include <sstream>                        // String       Stream: dump file contents into a string using `std::stringstream`.
#include <fstream>                        // File         Stream: read (.vert, .frag) files        using `std::ifstream`    .
#include <iostream>                       // Input/Output Stream: output error messages            using `std::cerr`        .

#include <glad/gl.h>                      // GLAD: OpenGL function pointers (e.g.: `glCreateShader()`).


// Reads a whole text file into a string. Returns an empty string on failure.
inline std::string readFile(const char* path) {
	std::ifstream file(path);
	if ( !file ) {
		std::cerr << "Could not open file: " << path << std::endl;
		return "";
	}
	
	std::stringstream buffer;
	buffer << file.rdbuf();  // Dump the file's entire contents into the buffer.
	return buffer.str();
}

// Both vertex and fragment shaders have identical compilation rituals.
// To avoid repetition, we define a compilation helper function below.
inline GLuint compileShader(
	GLenum      type  ,  // GL_VERTEX_SHADER or GL_FRAGMENT_SHADER.
	const char* source   // String (GLSL source code) to hand to the driver.
) {
	GLuint shader = glCreateShader(type);
	glShaderSource(
		shader ,  // PARAM.: ID of the shader object to fill.
		
		1      ,  // PARAM.: Number of GLSL source code strings to inject.
		          // ARG.  : We use just the 1: the contents of the .vert or .frag file.

		&source,  // PARAM.: An array of strings (type: const char**).
		          // ARG.  : We use a 1-element array: &source.
		
		nullptr   // PARAM.: An array of string lengths.
		          // ARG.  : nullptr = "measure them yourself, they are \0 terminated."
	);
	
	glCompileShader(shader);
	// Manage shader compilation errors.
	GLint success_compiling;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success_compiling);
	if ( !success_compiling ) {
		char log[1024];
		glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
		std::cerr << "Shader compilation failed:\n" << log << std::endl;
		glDeleteShader(shader);
		return 0;  // Shader ID 0 is reserved for "absence of shader".
	}
	
	return shader;
}

// Every shader program (e.g.: one for models, one for the skybox) is built the same way.
// Reads, compiles and links a vertex + fragment shader pair. Returns 0 on failure.
inline GLuint createProgram(const char* vertexPath, const char* fragmentPath) {
	std::string   vertexSource = readFile(  vertexPath);
	std::string fragmentSource = readFile(fragmentPath);
	if ( vertexSource.empty() || fragmentSource.empty() ) {
		return 0;
	}

	// `.c_str()` exposes the string's contents as the `const char*` OpenGL expects.
	GLuint   vertexShader = compileShader(GL_VERTEX_SHADER  ,   vertexSource.c_str());
	GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource.c_str());
	if ( vertexShader == 0 || fragmentShader == 0 ) {
		glDeleteShader(  vertexShader);  // Deleting shader 0 is silently ignored by OpenGL.
		glDeleteShader(fragmentShader);
		return 0;
	}

	// Link both stages into one program the GPU can run.
	GLuint program = glCreateProgram();
	glAttachShader(program,   vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	// The program keeps its own copy: the individual stages are no longer needed.
	glDeleteShader(  vertexShader);
	glDeleteShader(fragmentShader);

	GLint success_linking;
	glGetProgramiv(program, GL_LINK_STATUS, &success_linking);
	if ( !success_linking ) {
		char log[1024];
		glGetProgramInfoLog(program, sizeof(log), nullptr, log);
		std::cerr << "Shader program linking failed:\n" << log << std::endl;
		glDeleteProgram(program);
		return 0;  // Program ID 0 is reserved for "absence of program".
	}

	return program;
}
