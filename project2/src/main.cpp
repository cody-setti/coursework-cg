// --- INCLUSIONS ---
// CLASSIC C LIBRARIES
#include <cstdlib>                        // C Standard Library:
                                          // Provides human-readable exit codes `EXIT_SUCCESS` and `EXIT_FAILURE`.

// CLASSIC C++ LIBRARIES
#include <iostream>                       // Input/Output Stream: output error messages using `std::cerr`.

// COMPUTER GRAPHICS LIBRARIES
#include <glad/gl.h>                      // GLAD: loads driver-specific OpenGL function pointers at runtime.
                                          // Must be included before GLFW.

#include <GLFW/glfw3.h>                   // GL FrameWork: creates an OS window for OpenGL to draw into.

#include <glm/glm.hpp>                    // GL Mathematics: provides GLSL-compatible vector and matrix types.
#include <glm/ext/matrix_transform.hpp>   // e.g.: `translate()`, `rotate()`, `scale()`, `lookAt()`.
#include <glm/ext/matrix_clip_space.hpp>  // e.g.: `perspective()`.

// PROJECT HEADERS
#include "loading/shader.hpp"             // Building shader programs from GLSL source files.
#include "scene/model.hpp"                // Uploading textured models to the GPU and drawing them.
#include "scene/skybox.hpp"               // Uploading the sky cube to the GPU and drawing it.
#include "scene/camera.hpp"               // Camera state and View matrix.
#include "input.hpp"                      // Keys that act on the scene.


// --- CONSTANTS ---
namespace my {

// `constexpr` is a constant whos value can be resolved at compile time.
constexpr int WINDOW_WIDTH  = 1280;
constexpr int WINDOW_HEIGHT =  720;

// OBS.: Paths are relative to the directory the program is RUN from (the project root).
const char*  MODEL_VERT_PATH = "shaders/textured.vert";
const char*  MODEL_FRAG_PATH = "shaders/textured.frag";

const char* SKYBOX_VERT_PATH = "shaders/skybox.vert"  ;
const char* SKYBOX_FRAG_PATH = "shaders/skybox.frag"  ;

// Sky faces, in `my::loadCubemap()`'s order: +X, -X, +Y, -Y, +Z, -Z.
const char* const SKYBOX_FACE_PATHS[6] = {
	"assets/skybox/right.png" ,
	"assets/skybox/left.png"  ,
	"assets/skybox/top.png"   ,
	"assets/skybox/bottom.png",
	"assets/skybox/front.png" ,
	"assets/skybox/back.png"  ,
};

// External     models file paths.
const char*       GRASS_OBJ_PATH = "assets/grass/grass.obj"  ;
const char*        PINE_OBJ_PATH = "assets/pine/pine.obj";
const char*         OAK_OBJ_PATH = "assets/oak/oak.obj"  ;
// Transitional model  file path.
const char*         HUT_OBJ_PATH = "assets/hut/hut.obj"  ;

// Uniform locations, as fixed by the `layout (location = ...)` qualifiers in "shaders/textured.vert".
// OBS.: "shaders/skybox.vert" uses the same slots for V and P.
constexpr GLint MODEL_LOCATION = 0;
constexpr GLint  VIEW_LOCATION = 1;
constexpr GLint  PROJ_LOCATION = 2;

}  // namespace my


// --- MAIN ---
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
		my::WINDOW_WIDTH  ,
		my::WINDOW_HEIGHT ,
		"Spooky House"    ,  // PARAM.: Window title.
		nullptr           ,  // ARG.  : No fullscreen monitor.
		nullptr              // ARG.  : Don't share resources with another context.
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
		1 // ARG.: Synchronize buffer swap with monitor refresh rate (e.g.: 60 Hz)
		  //       to avoid thousands of unnecessary drawings that aren't displayed.
	);
	
	// Register the key callbacks GLFW will call during `glfwPollEvents()`.
	my::setupInput(window);
	
	
	// --- BUILD SHADER PROGRAMS ---
	GLuint shaderProgram = my::createProgram(my::MODEL_VERT_PATH , my::MODEL_FRAG_PATH );
	GLuint skyboxProgram = my::createProgram(my::SKYBOX_VERT_PATH, my::SKYBOX_FRAG_PATH);
	if ( shaderProgram == 0 || skyboxProgram == 0 ) {
		glfwTerminate();
		return EXIT_FAILURE;
	}
	
	
	// --- MODELS + SKYBOX ---
	// External models.
	my::Model  grass = my::loadModel (my::GRASS_OBJ_PATH   );
	my::Model  pine  = my::loadModel (my::PINE_OBJ_PATH    );
	my::Model  oak   = my::loadModel (my::OAK_OBJ_PATH     );
	// Transitional model.
	my::Model  hut   = my::loadModel (my::HUT_OBJ_PATH     );
	my::Skybox sky   = my::loadSkybox(my::SKYBOX_FACE_PATHS);
	if (
		 grass.VAO == 0 || pine.VAO == 0 || oak.VAO == 0 ||
		   hut.VAO == 0 ||
		   sky.VAO == 0
	) {
		glfwTerminate();
		return EXIT_FAILURE;
	}
	
	
	// --- RENDER LOOP ---
	glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	// The sky is drawn at depth 1.0, exactly the value `glClear()` resets the depth buffer to.
	// The default test (`GL_LESS`) would discard it there: `GL_LEQUAL` lets "equally far" pass too.
	glDepthFunc(GL_LEQUAL);
	
	my::Camera camera = {
		glm::vec3(0.0f, 8.0f, 12.0f)                                              ,  // Position.
		glm::normalize(glm::vec3(0.0f, 3.0f, 0.0f) - glm::vec3(0.0f, 8.0f, 12.0f)),  // Front: toward the point (0, 3, 0).
		glm::vec3(0.0f, 1.0f,  0.0f)                                                 // Up.
	};

	glm::mat4 P = glm::perspective(
		glm::radians(60.0f)                               , // Vertical FoV.
		(float)my::WINDOW_WIDTH / (float)my::WINDOW_HEIGHT, // Aspect ratio (window width/height).
		  0.1f                                            , // Near plane.
		100.0f                                              // Far  plane.
	);
	
	// Uniform values are stored PER PROGRAM: both programs need their own copy of P (and of V, in the loop).
	// OBS.: `glUniform*()` writes into the program currently in use.
	// P never changes: upload it once.
	glUseProgram(shaderProgram);
	glUniformMatrix4fv(my::PROJ_LOCATION, 1, GL_FALSE, &P[0][0]);
	
	glUseProgram(skyboxProgram);
	glUniformMatrix4fv(my::PROJ_LOCATION, 1, GL_FALSE, &P[0][0]);
	
	float lastTime = (float)glfwGetTime();
	
	while ( !glfwWindowShouldClose(window) ) {
		// Reset pixel values in the frame buffer before drawing a new frame.
		// Prevent garbage from previous frames corrupting the current drawing.
		glClear(
			GL_COLOR_BUFFER_BIT |  // Paint every pixel with the color specified in `glClearColor()`.
			GL_DEPTH_BUFFER_BIT    // Reset every pixel's depth to "infinitely far".
		);
		
		
		// PROCESS KEY PRESSES, CLICKS, ETC...
		glfwPollEvents();
		
		float currentTime = (float)glfwGetTime();
		float deltaTime   = currentTime - lastTime;
		lastTime          = currentTime;
		
		my::updateInput(camera, window, deltaTime);
		
		// V changes whenever the camera does: rebuild and upload it every frame.
		glm::mat4 V = my::viewMatrix(camera);
		glUseProgram(shaderProgram);
		glUniformMatrix4fv(my::VIEW_LOCATION, 1, GL_FALSE, &V[0][0]);
		glUseProgram(skyboxProgram);
		glUniformMatrix4fv(my::VIEW_LOCATION, 1, GL_FALSE, &V[0][0]);
		
		
		// MODELS
		glUseProgram(shaderProgram);
		glm::mat4 M;

		M = glm::mat4(1.0f);
		glUniformMatrix4fv(my::MODEL_LOCATION, 1, GL_FALSE, &M[0][0]);
		my::drawModel(grass);
		
		M = glm::mat4(1.0f);
		M = glm::translate(M,                            glm::vec3(-9.0f, 2.0f, -12.0f));
		M = glm::rotate   (M, 5.0f*(float)glfwGetTime(), glm::vec3(0.0f, 1.0f,  0.0f));
		glUniformMatrix4fv(my::MODEL_LOCATION, 1, GL_FALSE, &M[0][0]);
		my::drawModel(pine);
		
		M = glm::mat4(1.0f);
		M = glm::translate(M,                            glm::vec3(5.0f, 2.78f, -3.7f));
		M = glm::rotate   (M, 2.0f*(float)glfwGetTime(), glm::vec3( 0.0f, 1.0f,   0.0f));
		glUniformMatrix4fv(my::MODEL_LOCATION, 1, GL_FALSE, &M[0][0]);
		my::drawModel(oak);
		
		M = glm::mat4(1.0f);
		M = glm::translate(M,                            glm::vec3(0.0f, 2.1f, 0.0f));
		M = glm::rotate   (M, 0.3f*(float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(my::MODEL_LOCATION, 1, GL_FALSE, &M[0][0]);
		my::drawModel(hut);
		
		
		// SKY BOX
		// Drawn last: wherever a model already stands, the depth test discards the sky's (farther) fragments,
		// saving the work of shading them.
		glUseProgram(skyboxProgram);
		my::drawSkybox(sky);
		
		// DISPLAY FRAME
		// Show the finished frame (back buffer becomes front buffer).
		glfwSwapBuffers(window);
	}
	
	// --- SHUTDOWN ---
	my::deleteModel (grass);
	my::deleteModel (pine );
	my::deleteModel (oak  );
	my::deleteModel (hut  );
	my::deleteSkybox(sky  );
	
	glDeleteProgram(shaderProgram);
	glDeleteProgram(skyboxProgram);
	
	glfwTerminate();
	
	return EXIT_SUCCESS;
}
