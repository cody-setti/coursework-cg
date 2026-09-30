#include <cstdlib>       // C standard library  (e.g.: EXIT_SUCCESS, EXIT_FAILURE).
#include <iostream>      // Input/Output stream (e.g.: std::cout, std::cerr, std::endl).

#include <glad/gl.h>     // Loads driver-specific OpenGL function pointers at runtime.
                         // Must be included before GLFW.

#include <GLFW/glfw3.h>  // GL FrameWork: creates an OS window for OpenGL to draw into.


// --- SHADER SOURCES ---
// Shaders written in GLSL (GL Shading Language, unsurprisingly).
// Compiled by the GPU driver at runtime, not by g++.

// Runs once per vertex: decides WHERE the vertex lands on screen.
const char* VERTEX_SHADER_SOURCE = R"(
	#version 460 core
	layout (location = 0) in vec3 aPos;  // Vertex position, read from attribute slot 0.

	void main() {
		gl_Position = vec4(aPos, 1.0);   // No transformation yet: pass position through.
	}
)";

// Runs once per fragment: decides WHAT COLOR the fragment is.
const char* FRAGMENT_SHADER_SOURCE = R"(
	#version 460 core
	out vec4 FragColor;

	void main() {
		FragColor = vec4(1.0, 0.5, 0.2, 1.0);  // Opaque orange (RGBA).
	}
)";


// Both vertex and fragment shaders have identical compilation rituals.
// To avoid repetition, we define a compilation helper function below.
GLuint compileShader(
	GLenum type       ,  // GL_VERTEX_SHADER or GL_FRAGMENT_SHADER.
	const char* source   // String (GLSL source code) to hand to the driver.
) {
	GLuint shader = glCreateShader(type);
	glShaderSource(
		shader ,  // PARAM.: ID of the shader object to fill.
		
		1      ,  // PARAM.: Number of GLSL source code strings to inject.
		          // ARG.  : We use just the 1:         VERTEX or  FRAGMENT_SHADER_SOURCE.
		
		&source,  // PARAM.: An array of strings (type: const char**).
		          // ARG.  : We use a 1-element array: &VERTEX or &FRAGMENT_SHADER_SOURCE.
		
		nullptr   // PARAM.: An array of string lengths.
		          // ARG.  : nullptr = "measure them yourself, they are \0 terminated."
	);
	glCompileShader(shader);

	// Manage shader compilation errors.
	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if ( !success ) {
		char log[1024];
		glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
		std::cerr << "Shader compilation failed:\n" << log << std::endl;
		glDeleteShader(shader);
		return 0;
	}
	
	return shader;
}


int main() {
	// --- OPENGL CONTEXT CREATION ---
	// `glfwInit()` connects GLFW to the display system of the SPECIFIC
	// machine that will display the graphics described in this code.
	if ( !glfwInit() ) {
		std::cerr << "GLFW failed to initialize." << std::endl;
		return EXIT_FAILURE;
	}
	
	// Hints specify the configurations we want for our OpenGL context upon window creation.
	//             int hint                  , int value
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4                       );  // Request OpenGL version 4.x.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6                       );  // Request OpenGL version x.6.
	glfwWindowHint(GLFW_OPENGL_PROFILE       , GLFW_OPENGL_CORE_PROFILE);  // Explicitly disable deprecated legacy functions.
	
	GLFWwindow* window = glfwCreateWindow(
		800           ,  // Window width.
		800           ,  // Window height.
		"Spooky House",  // Window title.
		nullptr       ,  // No fullscreen monitor.
		nullptr          // Don't share resources with another context.
	);
	if ( window == nullptr ) {
		std::cerr << "Window creation failed." << std::endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}

	// --- OPENGL FUNCTION CALLS ---
	// Set "Spooky House" as context for OpenGL calls.
	glfwMakeContextCurrent(window);
	
	// Ask the driver for the address of every OpenGL function.
	if ( !gladLoadGL(glfwGetProcAddress) ) {
		std::cerr << "GLAD failed to load OpenGL functions." << std::endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}
	
	// OBS.: GLFW uses double buffering to avoid displaying half-drawn frames.
	glfwSwapInterval(
		1 // Synchronize buffer swap with monitor refresh rate (e.g.: 60 Hz)
		  // to avoid thousands of unnecessary drawings that aren't displayed.
	);


	// --- BUILD SHADER PROGRAM ---
	GLuint vertexShader   = compileShader(GL_VERTEX_SHADER  , VERTEX_SHADER_SOURCE  );
	GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, FRAGMENT_SHADER_SOURCE);
	if ( vertexShader == 0 || fragmentShader == 0 ) {
		glfwTerminate();
		return EXIT_FAILURE;
	}

	// Link both stages into one program the GPU can run.
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader  );
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	// The program keeps its own copy: the individual stages are no longer needed.
	glDeleteShader(vertexShader  );
	glDeleteShader(fragmentShader);

	GLint success;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if ( !success ) {
		char log[1024];
		glGetProgramInfoLog(shaderProgram, sizeof(log), nullptr, log);
		std::cerr << "Shader program linking failed:\n" << log << std::endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}


	// --- RENDER LOOP ---
	glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	
	while ( !glfwWindowShouldClose(window) ) {
		// Process key presses, clicks, etc...
		glfwPollEvents();
		
		// Reset the values in the buffer before drawing a new frame.
		// Prevent garbage from previous frames corrupting the current drawing.
		glClear(
			GL_COLOR_BUFFER_BIT |  // Paint every pixel with the color specified in `glClearColor()`.
			GL_DEPTH_BUFFER_BIT    // Reset every pixel's depth to "infinitely far"
		);

		glUseProgram(shaderProgram);  // Nothing to draw with it yet (next step: VBO/VAO).

		// Show the finished frame (back buffer becomes front buffer).
		glfwSwapBuffers(window);
	}

	// --- SHUTDOWN ---
	glDeleteProgram(shaderProgram);
	glfwTerminate();
	return EXIT_SUCCESS;
}
