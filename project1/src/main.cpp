// =============================================================================
//  CHEESE, PLEASE  --  Computacao Grafica, Projeto 1
//
//  CONTROLS
//    arrows   walk the rat around the tabletop
//    F        leap, with a full forward somersault on the way over
//    A / D    turn the whole mousetrap on the spot
//    W / S    swing the trap's hammer open and shut
//    Z / X    trim the cheese back, or put out a bigger piece
//    Q / E    turn the whole table, and everything standing on it, on the spot
//    T / G    tip the table towards Top or Ground
//    P        show or hide the polygon mesh
//    Esc      quit
// =============================================================================

// --- INCLUSIONS ---
#include <glad/gl.h>                 // GLAD = Multi-Language GL/GLES/EGL/GLX/WGL Loader-Generator.
                                     // 	> GLES = OpenGL for Embedded Systems
                                     // 	> EGL  = Embedded-System Graphics Library
                                     // 	> GLX  = OpenGL Extension to the X Window System
                                     // 	> WGL  = (MS) Windows Graphics Library
                                     // Load OS-specific OpenGL function pointers at runtime.

#include <GLFW/glfw3.h>              // GLFW = Graphics Library FRAMEWORK.
                                     // Create an OS window and capture keyboard inputs.

#include <glm/glm.hpp>               // GLM = OpenGL Mathematics.
                                     // Provide GLSL-compatible vector and matrix types.
                                     // 	> GLSL = OpenGL Shading Language

#include <glm/gtc/type_ptr.hpp>      // GTC = GLM Template Core.
                                     // Convert C++ GLM matrix objects into flat memory
                                     // arrays for OpenGL.
                                     // (The matrices themselves are built in
                                     //  primary_transforms.hpp, so none of GLM's
                                     //  transformation helpers are needed.)

#include <algorithm>                 // For std::min, to keep one slow frame from
                                     // teleporting the rat across the table.
#include <cstddef>                   // For offsetof, used to describe the vertex layout.
#include <iostream>                  // iostream = Input/Output Stream.
                                     // Report compilation and start-up errors to the terminal.

#include "geometry.hpp"
#include "scene.hpp"


// --- HANDLE DRIVER ERRORS ---
void APIENTRY debug_callback(
	[[maybe_unused]] GLenum        source   ,  // Who raised the callback (the API, the shader compiler, the window system, ...).
	[[maybe_unused]] GLenum        type     ,  // What kind it is (error, deprecation, performance warning, ...).
	[[maybe_unused]] GLuint        id       ,  // Driver-specific message number.
	                 GLenum        severity ,  // (HIGH, MEDIUM, LOW or NOTIFICATION)
	[[maybe_unused]] GLsizei       length   ,  // Length of `message`, in characters.
	                 const GLchar* message  ,  // The human-readable text. The most important parameter.
	[[maybe_unused]] const void*   userParam   // Whatever pointer we gave glDebugMessageCallback. Unused here.
) {
	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;  // Mute chatty "buffer allocated" notices.
	std::cerr << "[OpenGL] " << message << std::endl;
}


// --- KEEP THE PICTURE SQUARE, WHATEVER SHAPE THE WINDOW IS ---
void framebuffer_size_callback([[maybe_unused]] GLFWwindow* window, int width, int height) {
	const int side = std::min(width, height);
	glViewport(
		(width  - side) / 2,  // Left edge of the rectangle we draw into, in pixels.
		(height - side) / 2,  // Bottom edge. OpenGL counts upwards from the bottom.
		side               ,  // Width ...
		side                  // ... and height. A square, centred in the window.
	);
}


// --- DEFINE SHADERS ---
// String storing GLSL source code.
// C++ doesn't compile it, the GPU driver does (at runtime).
const char* vertexShaderSource = R"(
#version 460 core                                    // Specify the version of OpenGL/GLSL in use.
                                                     // The `core` profile excludes legacy commands.
// Declare vertex shader interface.
// Its plumbing: what flows in, what flows out and from where.

// QUALIFIER - CHANGES...
// uniform   - once per draw call
// in        - once per vertex
// out       - once per vertex (blended per fragment later)
layout (location = 0) in vec3 aPos;                  // Read slot 0 - same 0 as glVertexArrayAttribFormat.
layout (location = 1) in vec3 aColor;                // Read slot 1 - same 1 as glVertexArrayAttribFormat.
                                                     // `aColor` is a `vec3` because of RGB (opacity added later).
out vec3 vertexColor;                                // Hand the colour on to the fragment shader.
uniform mat4 transform;                              // This object's own transformation matrix,
                                                     // sent from the CPU before each draw call.

void main() {
    gl_Position = transform * vec4(aPos, 1.0);       // `w=1.0` means "a point", so the matrix's
                                                     // translation column applies to it.
    vertexColor = aColor;                            // Pass the vertex's colour down the pipeline.
}
)";

// String storing GLSL source code.
// C++ doesn't compile it, the GPU driver does (at runtime).
const char* fragmentShaderSource = R"(
#version 460 core
in  vec3 vertexColor;                                // The colour, smoothly blended between the
                                                     // three vertices of the triangle this
                                                     // fragment fell inside.
out vec4 FragColor;                                  // The final colour of the fragment (a
                                                     // candidate for a screen pixel).

void main() {
    FragColor = vec4(vertexColor, 1.0);              // Add alpha channel. It does nothing here - blending is off.
}
)";


// --- HELPER FUNCTION FOR SHADER BUILDING ---
// Both shaders (vertex and fragment) go through an identical create/inject/compile/check
// ritual, so it is written once here rather than twice inside main().
unsigned int compile_shader(
	GLenum      type  ,  // GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
	const char* source   // The GLSL string to hand to the driver.
) {
	// OpenGL objects live in the driver (GPU, usually) memory. We only ever hold their ID numbers in the CPU, however.
	unsigned int shader = glCreateShader(type);  // Create an empty SHADER object and get its ID.
	glShaderSource(
		shader ,  // ID of the shader object to fill.
		1      ,  // Number of GLSL source code strings to inject. Here, a single string.
		&source,  // The array of strings. Here, the address (a `const char**`) of our one string.
		nullptr   // An array of string lengths. nullptr = "measure them yourself, they are \0 terminated."
	);
	glCompileShader(shader);                     // Compile the GLSL into this specific GPU's machine code.

	int success;                                 // Result flag for the compile check.
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {                              // If an error occurred, inform it.
		char infoLog[512];
		glGetShaderInfoLog(
			shader ,
			512    ,  // The size of YOUR buffer, `infoLog[512]`
			nullptr,  // nullptr = "don't bother telling me how many characters you actually wrote."
			infoLog   // Variable in which the error message is injected.
		);
		std::cerr << "Shader failed:\n" << infoLog << std::endl;
		glDeleteShader(shader);
		return 0;
	}
	return shader;                               // Hand the ID back so main() can link it.
}


int main() {
	// --- WINDOW / CONTEXT SETUP ---
	// Initialize GLFW: connect to the window system, find monitor(s), etc.
	if (!glfwInit()) {
		std::cerr << "GLFW failed to initialize." << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);                  // Request OpenGL version 4.x.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);                  // Request OpenGL version x.6.
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // Explicitly disable deprecated legacy functions.
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);           // Permit `debug_callback()` to fire.
	glfwWindowHint(GLFW_DEPTH_BITS, 24);                            // Ask for a depth buffer: the scene is 3D, and
	                                                                // something has to decide which surface is in front.

	GLFWwindow* window = glfwCreateWindow(  // Allocate memory and ask the OS for a window.
		800             ,  // Window width.
		800             ,  // Window height.
		"Cheese, Please",  // Window name.
		nullptr         ,  // No fullscreen monitor.
		nullptr            // Don't share resources with another context.
	);
	if (window == nullptr) {
		std::cerr << "Window creation failed." << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwSetWindowAspectRatio(window, 1, 1);
	glfwMakeContextCurrent(window);  // Every later gl* call now talks to this window's context.
	glfwSwapInterval(1);             // Show a new frame only when the monitor is ready for one:
	                                 // no tearing, and no redrawing three objects 5000 times a second.
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {  // Hand GLAD GLFW's lookup function so it can find every gl*.
		std::cerr << "GLAD failed to load OpenGL." << std::endl;
		glfwTerminate();
		return -1;
	}                                                     //	`glfwGetProcAddress` is cast to `GLADloadfunc`,
	                                                      //	 because GLAD and GLFW use slightly different types
	                                                      //	 for retrieving procedure addresses.

	// --- TURN ON ERROR REPORTING ---
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);  // Report each problem while the offending call is still
	                                        // on the stack, instead of whenever the driver gets round to it.
	glDebugMessageCallback(
		debug_callback,  // Invoke our custom-built function on every message.
		nullptr          // Optional pointer passed back to us as `userParam`. We need none.
	);

	// The resize callback only fires on a CHANGE, so set the viewport up by hand for
	// whatever size the window actually opened at.
	int fbWidth, fbHeight;
	glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
	framebuffer_size_callback(window, fbWidth, fbHeight);

	// --- BUILD AND LINK SHADERS ---
	unsigned int vertexShader   = compile_shader(GL_VERTEX_SHADER  , vertexShaderSource  );
	unsigned int fragmentShader = compile_shader(GL_FRAGMENT_SHADER, fragmentShaderSource);
	if (vertexShader == 0 || fragmentShader == 0) {  // Nothing below this line can work
		glfwDestroyWindow(window);                   // without both of them.
		glfwTerminate();
		return -1;
	}

	unsigned int shaderProgram = glCreateProgram();  // Create an empty PROGRAM object and get its ID.
	glAttachShader(shaderProgram, vertexShader  );   // Attach the compiled vertex   logic to `shaderProgram`, via IDs.
	glAttachShader(shaderProgram, fragmentShader);   // Attach the compiled fragment logic to `shaderProgram`, via IDs.
	glLinkProgram(shaderProgram);                    // Resolve inputs/outputs between the vertex and fragment shaders.

	int success;                                     // Result flag for the link check.
	char infoLog[512];                               // Buffer for the driver's error message.
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {                                  // If an error occurred, inform it and stop:
		glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
		std::cerr << "Shader program linking failed:\n" << infoLog << std::endl;
		glDeleteProgram(shaderProgram);              // there would be nothing to draw with.
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}
	glDeleteShader(vertexShader);                    // Flag for deletion: the individual shaders are no
	glDeleteShader(fragmentShader);                  // longer needed once they have been linked.

	// --- BUILD THE SCENE ON THE CPU ---
	// Five objects, six independently moving pieces, one buffer.
	SceneMesh   mesh;
	Scene       scene    = build_scene(mesh);
	const Mesh& vertices = mesh.vertices;

	// --- SEND THE GEOMETRY TO THE GPU ---
	unsigned int VBO;                    // VBO = Vertex Buffer Object.
	                                     // A dumb bucket of bytes. It will hold every vertex of
	                                     // the scene, but knows nothing about them.
	glCreateBuffers(                     // Generate an ID *and* the buffer object behind it.
		1   ,  // The number of desired buffers.
		&VBO   // The address where to write each ID.
	);
	glNamedBufferStorage(                // Allocate and fill the buffer, naming it instead of binding it.
		VBO                                                       ,  // Which buffer.
		static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)) ,  // How many bytes.
		vertices.data()                                           ,  // The address to copy from.
		0                                                            // Flags. `0` = immutable: never resized,
		                                                             // never rewritten. The objects deform
		                                                             // only through their matrices, so the
		                                                             // vertices genuinely never change.
	);

	// DESCRIBE THE BYTES (VBO & VAO)
	unsigned int VAO;                         // VAO = Vertex Array Object.
	                                          // The RECIPE. No vertex data, just brains.
	glCreateVertexArrays(1, &VAO);            // Generate an ID *and* the object for our state-tracking recipe.
	glVertexArrayVertexBuffer(                // Plug the VBO into one of the VAO's "binding points".
		VAO           ,  // The recipe being written.
		0             ,  // Binding point `0`. An arbitrary shelf number, unrelated to attribute numbers.
		VBO           ,  // The buffer that sits on that shelf.
		0             ,  // Byte offset of the first vertex inside the buffer.
		sizeof(Vertex)   // Stride: one vertex every six floats, position and colour together.
	);

	glEnableVertexArrayAttrib(VAO, 0);        // Explicitly turn on the POSITION numbered input slot.
	glVertexArrayAttribFormat(                // Describe the SHAPE of attribute 0. Says nothing about which buffer.
		VAO                        ,
		0                          ,  // Attribute `0` (ties back to `aPos` in the vertex shader)
		3                          ,  // has `3`
		GL_FLOAT                   ,  // `floats`.
		GL_FALSE                   ,  // Don't rescale them.
		offsetof(Vertex, position)    // and they start at the top of each vertex.
	);
	glVertexArrayAttribBinding(VAO, 0, 0);    // Feed attribute `0` from binding point `0`.

	glEnableVertexArrayAttrib(VAO, 1);        // Explicitly turn on the COLOR numbered input slot.
	glVertexArrayAttribFormat(                // Describe the SHAPE of attribute 1.
		VAO                     ,
		1                       ,  // Attribute `1` (ties back to `aColor`)
		3                       ,  // has `3`
		GL_FLOAT                ,  // `floats`.
		GL_FALSE                ,  // Don't rescale them.
		offsetof(Vertex, color)    // sitting just after the position. Asking the compiler where the
		                           // field is beats counting bytes by hand and being wrong later.
	);
	glVertexArrayAttribBinding(VAO, 1, 0);    // Feed attribute `1` from the same binding point `0`.

	// --- WIRE UP THE KEYBOARD ---
	glfwSetWindowUserPointer(window, &scene);  // Let the callback find the scene without a global variable.
	glfwSetKeyCallback(window, key_callback);  // Register our custom input function with GLFW's event system.

	// --- STANDING GPU STATE ---
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); 

	GLint transformLoc = glGetUniformLocation(shaderProgram, "transform");

	// Draw one piece of the scene under its own transformation matrix.
	auto draw = [&](const Piece& piece, const glm::mat4& transform) {
		glProgramUniformMatrix4fv(      // Send the matrix from CPU to GPU before drawing.
			shaderProgram            ,  // Naming the program means this works without glUseProgram first.
			transformLoc             ,  // Where the uniform goes.
			1                        ,  // Number of matrices being passed.
			GL_FALSE                 ,  // Don't transpose (GLM is column-major, and so is GLSL).
			glm::value_ptr(transform)   // Pointer to the 16 floats in CPU memory.
		);
		glDrawArrays(
			GL_TRIANGLES,  // How to group vertices: three at a time, each trio a triangle.
			piece.first ,  // Where to start, counted in vertices.
			piece.count    // How many vertices to process.
		);
	};

	// --- RENDER LOOP ---
	float lastTime = static_cast<float>(glfwGetTime());
	while (!glfwWindowShouldClose(window)) {   // Keep the application running until an exit signal is received.
		const float now     = static_cast<float>(glfwGetTime());
		const float seconds = std::min(now - lastTime, 0.1f);
		lastTime            = now;

		update(scene, window, seconds);

		// APPLY WHAT THE VIEWER ASKED FOR
		glPolygonMode(                                    // Requirement 9.
			GL_FRONT_AND_BACK,
			scene.wireframe ? GL_LINE : GL_FILL           // Draw only the edges ("wires") between
		);                                                // vertices, or fill the polygons in.

		// CLEAR
		glClearColor(0.8f, 0.74f, 0.83f, 1.0f);           // Define the clear color for the window.
		glClear(
			GL_COLOR_BUFFER_BIT |                         // Overwrite pixels with the clear color, preventing visual trailing.
			GL_DEPTH_BUFFER_BIT                           // Forget how far away last frame's surfaces were, too.
		);

		// DRAW
		glUseProgram(shaderProgram);  // Instruct the GPU to use our specific custom shader program.
		glBindVertexArray(VAO);       // Recall all the data mapping configurations we set up earlier.

		for (const Part& part : scene.parts)
			draw(part.piece, part.transform(scene));

		glfwSwapBuffers(window);  // Push the newly drawn frame to the screen (double buffering).
		glfwPollEvents();         // Process pending OS events like window resizes or 'p' key presses.
	}

	// --- SHUTDOWN ---
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(shaderProgram);
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
