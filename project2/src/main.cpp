// --- INCLUSIONS ---
// CLASSIC C LIBRARIES
#include <cstdlib>                        // C Standard Library:
                                          // Provides human-readable exit codes `EXIT_SUCCESS` and `EXIT_FAILURE`.

// CLASSIC C++ LIBRARIES
#include <glm/trigonometric.hpp>
#include <iostream>                       // Input/Output Stream: output error messages using `std::cerr`.
#include <vector>

// COMPUTER GRAPHICS LIBRARIES
#include <glad/gl.h>                      // GLAD: loads driver-specific OpenGL function pointers at runtime.
                                          // Must be included before GLFW.

#include <GLFW/glfw3.h>                   // GL FrameWork: creates an OS window for OpenGL to draw into.

#include <glm/glm.hpp>                    // GL Mathematics: provides GLSL-compatible vector and matrix types.
#include <glm/ext/matrix_transform.hpp>   // e.g.: `translate()`, `rotate()`, `scale()`, `lookAt()`.
#include <glm/ext/matrix_clip_space.hpp>  // e.g.: `perspective()`.
#include <ostream>

// PROJECT HEADERS
#include "loading/shader.hpp"             // Building shader programs from GLSL source files.
#include "scene/model.hpp"                // Uploading textured models to the GPU and drawing them.
#include "scene/skybox.hpp"               // Uploading the sky cube to the GPU and drawing it.
#include "scene/camera.hpp"               // Camera state and View matrix.
#include "input.hpp"                      // Keys that act on the scene.
#include "scene/constants.hpp" 			  // Constants regarding the objects and their movimentations
#include "scene/scene.hpp"               // Struct scene for glfw's Window User Pointer


// --- CONSTANTS ---
namespace my {

// `constexpr` is a constant whos value can be resolved at compile time. 
// Used only for defining the first window size
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
const char* 	    POT_OBJ_PATH = "assets/pot/zelda_pot.obj";
const char* 	  HEART_OBJ_PATH = "assets/piece_of_heart/Piece_of_heart.obj";
const char* 	   MASK_OBJ_PATH = "assets/mask_salesman/Happy_Mask_Salesman.obj";
const char* 	   MOON_OBJ_PATH = "assets/moon/moon.obj";
const char*		  EPONA_OBJ_PATH = "assets/epona/epona.obj";
const char*		 SHIELD_OBJ_PATH = "assets/shield/shield.obj";

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
		"Falling Moon"    ,  // PARAM.: Window title.
		nullptr           ,  // ARG.  : No fullscreen monitor.
		nullptr              // ARG.  : Don't share resources with another context.
	);
	if ( window == nullptr ) {
		std::cerr << "Window creation failed." << std::endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}
	
	// --- OPENGL FUNCTION CALLS ---
	// Set our window as context for OpenGL calls.
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
	my::Model  grass   = my::loadModel (my::GRASS_OBJ_PATH   );
	my::Model  pine    = my::loadModel (my::PINE_OBJ_PATH    );
	my::Model  oak     = my::loadModel (my::OAK_OBJ_PATH     );
	my::Model pot      = my::loadModel(my::POT_OBJ_PATH      );
	my::Model salesman = my::loadModel(my::MASK_OBJ_PATH     );
	my::Model heart    = my::loadModel(my::HEART_OBJ_PATH    );
	my::Model moon     = my::loadModel(my::MOON_OBJ_PATH     );
	my::Model epona    = my::loadModel(my::EPONA_OBJ_PATH    );
	my::Model shield   = my::loadModel(my::SHIELD_OBJ_PATH   );
	// Transitional model.
	my::Model  hut     = my::loadModel (my::HUT_OBJ_PATH     );
	my::Skybox sky     = my::loadSkybox(my::SKYBOX_FACE_PATHS);
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
		my::CAMERA_POS_INI  ,  // Position.
		my::CAMERA_FRONT_INI,  // Front
		my::CAMERA_UP          // Up.
	};

	my::Scene scene = {
		&camera
	}; //creates a scene with the camera we just created 

	glfwSetWindowUserPointer(window, static_cast<void*>(&scene));

	
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
		
		glPolygonMode(
			GL_FRONT_AND_BACK,
			scene.wireframe ? GL_LINE : GL_FILL           // Draw only the edges ("wires") between
		);     

		float currentTime = (float)glfwGetTime();
		float deltaTime   = currentTime - lastTime;
		lastTime          = currentTime;
		
		my::updateInput(camera, window, deltaTime);

		glm::mat4 P = glm::perspective(
			glm::radians(scene.fov)                               , // Vertical FoV.
			(float)scene.window_width / (float)scene.window_height, // Aspect ratio (window width/height).
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
		

		// V changes whenever the camera does: rebuild and upload it every frame.
		glm::mat4 V = my::viewMatrix(camera);
		glUseProgram(shaderProgram);
		glUniformMatrix4fv(my::VIEW_LOCATION, 1, GL_FALSE, &V[0][0]);
		glUseProgram(skyboxProgram);
		glUniformMatrix4fv(my::VIEW_LOCATION, 1, GL_FALSE, &V[0][0]);
		
		
		// MODELS
		glUseProgram(shaderProgram);
		glm::mat4 M;

		//GRASS
		M = glm::mat4(1.0f);
		M = glm::translate(M,                            my::GRASS_POS);
		M = glm::scale(M, my::GRASS_SCALE);
		glUniformMatrix4fv(my::MODEL_LOCATION, 1, GL_FALSE, &M[0][0]);
		my::drawModel(grass);
		
		//PINE
		for(int i = 0; i < my::PINE_POS.size(); i++){
			M = glm::mat4(1.0f);
			M = glm::translate(M,                          my::PINE_POS[i]);
			M = glm::scale(M, 							   my::PINE_SCALE[i]);
			glUniformMatrix4fv(my::MODEL_LOCATION, 1, GL_FALSE, &M[0][0]);
			my::drawModel(pine);
		}
		
		//OAK
		for(int i = 0; i < my::OAK_POS.size(); i++){
			M = glm::mat4(1.0f);
			M = glm::translate(M,                            my::OAK_POS[i]);
			M = glm::scale(M, 							   my::OAK_SCALE[i]);
			glUniformMatrix4fv(my::MODEL_LOCATION, 1, GL_FALSE, &M[0][0]);
			my::drawModel(oak);
		
		}
		
		//HUT
		M = glm::mat4(1.0f);
		M = glm::translate(M,                            my::HUT_POS);
		glUniformMatrix4fv(my::MODEL_LOCATION, 1, GL_FALSE, &M[0][0]);
		my::drawModel(hut);
		
		//SHIELD
		M = glm::mat4(1.0f);
		M = glm::translate(M,                            my::SHIELD_POS);
		M = glm::rotate(M, my::SHIELD_ANGLES.z, glm::vec3(0.0f, 0.0f, 1.0f));
		M = glm::rotate(M, my::SHIELD_ANGLES.y, glm::vec3(0.0f, 1.0f, 0.0f));
		M = glm::scale(M, my::SHIELD_SCALE);
		glUniformMatrix4fv(my::MODEL_LOCATION, 1, GL_FALSE, &M[0][0]);
		my::drawModel(shield);

		//MOON OBJECT
		M = glm::mat4(1.0f);
		M = glm::translate(M,                            my::MOON_POS);
		M = glm::scale(M, glm::vec3(scene.moon_scale));
		M = glm::rotate(M, glm::radians(-60.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		M = glm::rotate(M, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		M = glm::rotate(M, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(my::MODEL_LOCATION, 1, GL_FALSE, &M[0][0]);
		my:my::drawModel(moon);
		
		//SALESMAN
		M = glm::mat4(1.0f);
		M = glm::translate(M,                            my::MASK_POS);
		M = glm::scale(M, my::MASK_SCALE);
		glUniformMatrix4fv(my::MODEL_LOCATION, 1, GL_FALSE, &M[0][0]);
		my::drawModel(salesman);

		//POTS
		M = glm::mat4(1.0f);
		M = glm::translate(M,                            my::POT1_POS);
		M = glm::scale(M, my::POT_SCALE);
		glUniformMatrix4fv(my::MODEL_LOCATION, 1, GL_FALSE, &M[0][0]);
		my::drawModel(pot);

		M = glm::mat4(1.0f);
		M = glm::translate(M,                            my::POT2_POS);
		M = glm::scale(M, my::POT_SCALE);
		glUniformMatrix4fv(my::MODEL_LOCATION, 1, GL_FALSE, &M[0][0]);
		my::drawModel(pot);

		//HEART
		M = glm::mat4(1.0f);
		M = glm::translate(M,                            my::HEART_POS);
		M = glm::rotate(M, scene.heart_angle, glm::vec3(0.0f, 1.0f, 0.0f));
		M = glm::scale(M, my::HEART_SCALE);
		glUniformMatrix4fv(my::MODEL_LOCATION, 1, GL_FALSE, &M[0][0]);
		my::drawModel(heart);


		//EPONA
		M = glm::mat4(1.0f);
		M = glm::translate(M,                            scene.epona_pos);
		float theta = glm::atan(scene.epona_last_direction.z, scene.epona_last_direction.x);
		M = glm::rotate(M, -theta, glm::vec3(0.0f, 1.0f, 0.0f));
		M = glm::scale(M, my::EPONA_SCALE);
		M = glm::rotate(M, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(my::MODEL_LOCATION, 1, GL_FALSE, &M[0][0]);
		my::drawModel(epona);

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
