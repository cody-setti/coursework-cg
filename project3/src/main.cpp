// --- MAIN ---
// WHEN everything happens: at start-up, once per frame or at shutdown?

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


// PROJECT HEADERS
#include "loading/shader.hpp"             // Builds shader programs from GLSL source files.

#include "render/model.hpp"               // Uploads textured models to the GPU and draws them.
#include "render/skybox.hpp"              // Uploads the sky cube to the GPU and draws it.

#include "world/transform.hpp"            // Placement of model instances, and their M.
#include "world/scene.hpp"                // WHAT is in the world and WHERE it stands.
#include "world/camera.hpp"               // A flying eye, and its V and P.

#include "input.hpp"                      // Every keyboard and mouse read.


constexpr float TAU = 6.28318530718f;  // τ: one full turn, in radians.


int main() {
	
	// █▀ ▀█▀ ▄▀█ █▀█ ▀█▀ ▄▄ █ █ █▀█
	// ▄█  █  █▀█ █▀▄  █     █▄█ █▀▀
	// START-UP                     
	
	// --- WINDOW AND CONTEXT CREATION ---
	// `glfwInit()` connects GLFW to the display system of the SPECIFIC
	// machine that will display the graphics described in this code.
	if ( !glfwInit() ) {
		std::cerr << "GLFW failed to initialize." << std::endl;
		return EXIT_FAILURE                                   ;
	}
	
	// Hints specify the configurations we want for our OpenGL context upon window creation.
	//             int hint                  , int value
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4                       );  // Request OpenGL version 4.x.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6                       );  // Request OpenGL version x.6.
	glfwWindowHint(GLFW_OPENGL_PROFILE       , GLFW_OPENGL_CORE_PROFILE);  // Explicitly disable deprecated legacy functions.
	
	GLFWwindow* const window = glfwCreateWindow(  //  `type* const` sets the pointer's target in stone: Falling Moon.
	                                              //  `const type*` would make the window immutable.
		1920          ,  // PARAM.: Window width.
		1080          ,  // PARAM.: Window height.
		"Falling Moon",  // PARAM.: Window title.
		nullptr       ,  // ARG.  : No fullscreen monitor.
		nullptr          // ARG.  : Don't share resources with another context.
	);
	if ( window == nullptr ) {
		std::cerr << "Window creation failed." << std::endl;
		glfwTerminate()                                    ;
		return EXIT_FAILURE                                ;
	}
	// Set "Falling Moon" as context for OpenGL calls.
	glfwMakeContextCurrent(window);
	
	// OBS.: GLFW uses double buffering to avoid displaying half-drawn frames.
	glfwSwapInterval(
		1 // ARG.: Synchronize buffer swap with monitor refresh rate (e.g.: 60 Hz)
		  //       to avoid thousands of unnecessary drawings that aren't displayed.
	);
	
	
	// --- LOAD OPENGL FUNCTION CALLS ---
	// Ask the driver for the address of every OpenGL 4.6 function.
	if ( !gladLoadGL(glfwGetProcAddress) ) {
		std::cerr << "GLAD failed to load OpenGL functions." << std::endl;
		glfwTerminate()                                                  ;
		return EXIT_FAILURE                                              ;
	}
	
	
	// --- RENDER SETTINGS ---
	bool wireframe = false ;  // When set to `true`, models are displayed by their meshes.
	
	glEnable(GL_DEPTH_TEST);  // Overwrite a pixel only if the fragment's depth is lower (or equal) to the pixel's depth.
	glDepthFunc(GL_LEQUAL) ;  // The sky is drawn at depth 1.0, exactly the value `glClear()` resets the depth buffer to.
	                          // The default test (`GL_LESS`) would discard it there: `GL_LEQUAL` lets "equally far" pass too.
	
	
	// --- BUILD SHADER PROGRAMS ---
	// Shaders are quick to build and fail fast, meanwhile loading models is expensive.
	// For that reason, we build the shaders first.
	
	// OBS.: Paths are relative to the directory the program is RUN from (the project root).
	const GLuint modelProgram  = shdr::loadProgram("shaders/model.vert" , "shaders/model.frag" );
	const GLuint skyboxProgram = shdr::loadProgram("shaders/skybox.vert", "shaders/skybox.frag");
	if ( modelProgram == 0 || skyboxProgram == 0 ) {
		glfwTerminate()    ;
		return EXIT_FAILURE;
	}
	
	
	// --- CAMERA BUFFER ---
	// V and P live in ONE buffer THAT BELONGS TO THE CONTEXT. Therefore, both shader programs may read it (see the `Camera` block in their vertex shaders).
	// If V and P were stored as plain uniforms, AND, THEREFORE, BELONGED TO SPECIFIC SHADERS, we'd need one upload of each PER PROGRAM.
	GLuint cameraBuffer;
	glCreateBuffers(1, &cameraBuffer);
	glNamedBufferStorage(    // Creates IMMUTABLE storage. The size is fixed for good.
		cameraBuffer                              ,
		2 * sizeof(glm::mat4) + sizeof(glm::vec4) ,  // PARAM.: Buffer's size in bytes.
		                                             // ARG.  : V, then P, then the eye position.
		                                             // OBS.  : `std140` aligns a vec3 like a vec4 (16 bytes), and the
		                                             //         block's size is rounded up to match: 128 + 16 = 144.
		nullptr                                   ,  // ARG.  : No initial data: the render loop writes it.
		GL_DYNAMIC_STORAGE_BIT                       // ARG.  : Allow `glNamedBufferSubData()` to rewrite its contents later.
	);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, cameraBuffer);  // Hook it to binding point 0.
	
	
	// --- LOAD MODELS AND SKY BOX ---
	// Load every model used in the scene into GPU memory.
	mdl::Model models[scn::MODEL_COUNT];
	for ( int id = 0; id < scn::MODEL_COUNT; id++ ) {
		models[id] = mdl::loadModel(scn::OBJ_PATHS[id]);
		if ( models[id].VAO == 0 ) {
			std::cerr << "Scene aborted: failed to load model " << scn::OBJ_PATHS[id] << std::endl;
			glfwTerminate()    ;
			return EXIT_FAILURE;
		}
	}
	// Load the sky box used in the scene into GPU memory.
	sky::Skybox sky = sky::loadSkybox(scn::SKYBOX_FACE_PATHS);
	if ( sky.VAO == 0 ) {
		std::cerr << "Scene aborted: failed to load sky box." << std::endl;
		glfwTerminate()    ;
		return EXIT_FAILURE;
	}
	
	
	// --- WORLD STATE ---
	scn::Scene scene;
	scn::arrangeScene(scene);
	cam::Camera camera = cam::createCamera(scn::EYE_START, scn::TRGT_START);
	
	
	// --- INPUT ---
	input::Carryover carry;
	input::setUpInput(window, carry);
	
	
	// --- FRAME CLOCK ---
	// Must come last in START-UP, so that the first frame starts immediately after this time recording.
	double lastTime = glfwGetTime();
	// Incrementing the camera's (or a model's) position each frame by a constant Δx would make it
	// so it moves much faster in high-end monitors. Suppose Δx = 0.05 m, then:
	//                                                                     
	//  Monitor │ Distance walked in a frame (Δx) │ Distance walked in 1s  
	// ─────────┼─────────────────────────────────┼────────────────────────
	//   30 Hz  │              0.05 m             │  30 × 0.05   = 1.5 m   
	// ─────────┼─────────────────────────────────┼────────────────────────
	//  144 Hz  │              0.05 m             │ 144 × 0.05   = 7.2 m   
	//                                                                     
	// To be hardware agnostic, we use Δx = speed * Δt, where speed is constant but Δt is the
	// time between consecutive frames. Suppose speed = 3 m/s, then:
	//                                                                     
	//  Monitor │ Distance walked in a frame (Δx) │ Distance walked in 1s  
	// ─────────┼─────────────────────────────────┼────────────────────────
	//   30 Hz  │        3 × 1/30  = 0.1          │  30 × 0.1    = 3 m     
	// ─────────┼─────────────────────────────────┼────────────────────────
	//  144 Hz  │        3 × 1/144 ≈ 0.0208       │ 144 × 0.0208 = 3 m     
	//                                                                     
	// In high-end monitors, although frames come around more often, each one covers less time.
	//
	// As a matter of fact, since we are measuring frame-rate via clock readings, all works
	// well even for an unstable frame-rate.
	
	
	// █▀█ █▀▀ █▄ █ █▀▄ █▀▀ █▀█   █   █▀█ █▀█ █▀█
	// █▀▄ ██▄ █ ▀█ █▄▀ ██▄ █▀▄   █▄▄ █▄█ █▄█ █▀▀
	// RENDER LOOP                               
	
	while ( !glfwWindowShouldClose(window) ) {
		// --- FRAME TIME ---
		const double time = glfwGetTime();
		const float  dt   = glm::min((float)(time - lastTime), 0.1f);
		// OBS.: `dt` is capped at 0.1s because, otherwise, after any stall (e.g.: minimized window, dragged window)
		//       a single frame would carry the whole pause and teleport everything that moves.
		//       The only downside is that the scene runs in slow-mo at < 10 FPS. Unlikely for current hardware...
		lastTime = time;
		
		
		// --- POLL EVENTS ---
		glfwPollEvents();
		
		
		// --- FRAME BUFFER SIZE ---
		// The variables below permit dynamic window sizing each render.
		int fbWidth, fbHeight;
		glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
		// We measure the frame buffer dimensions, given in physical pixels, instead of the window
		// ones, given in logical units, because pixel density varies between monitors.
		
		// A minimized window reports 0×0: there is nothing to draw into.
		if ( fbWidth == 0 || fbHeight == 0 ) {
			glfwWaitEvents();  // Sleep until GLFW receives an event (e.g.: the window being restored),
			continue;          // then start the frame over.
		}
		
		
		// --- UPDATE WORLD ---
		// Input-dependent changes.
		input::processInput(window, carry, dt, scene, camera, wireframe);
		// Unconditional changes.
		scene.heart.angles.y += (0.3f * TAU) * dt;
		
		
		// --- CAMERA DATA ---
		const float     aspect            = (float)fbWidth / (float)fbHeight;
		const glm::mat4 cameraMatrices[2] = {
			// Same memory layout as the `std140` block in the vertex shader programs.
			cam::viewMatrix(camera)        ,  // Offset  0: V.
			cam::projMatrix(camera, aspect)   // Offset 64: P.
		};
		glNamedBufferSubData(cameraBuffer, 0                     , sizeof(cameraMatrices), cameraMatrices);
		glNamedBufferSubData(cameraBuffer, sizeof(cameraMatrices), sizeof(glm::vec3)     , &camera.eye   );
		
		
		// --- VIEWPORT ---
		// Map the drawing onto the whole frame buffer.
		glViewport(0, 0, fbWidth, fbHeight);
		
		
		// --- MESH INSPECTION ---
		glPolygonMode(  // Here, "polygon" means the triangle primitives models (and the sky box) are built from.
			GL_FRONT_AND_BACK            ,  // ARG.: Applies to triangles facing the camera AND facing away from it (i.e.: all triangles).
			                                // OBS.: `GL_FRONT_AND_BACK` is the only value core profile accepts;
			                                //       `GL_FRONT` and `GL_BACK` are ignored.
			
			wireframe ? GL_LINE : GL_FILL   // ARG.:
			                                // > `GL_LINE`: draw only the triangle edges.
			                                // > `GL_FILL`: draw their whole surface (the usual drawing mode).
		);
		
		
		// --- CLEAR FRAME BUFFER ---
		// Reset pixel values in the frame buffer before drawing a new frame.
		// Prevent garbage from previous frames corrupting the current drawing.
		glClear(
			GL_COLOR_BUFFER_BIT |  // Paint every pixel black.
			GL_DEPTH_BUFFER_BIT    // Reset every pixel's depth to "infinitely far".
		);
		
		
		// --- DRAW MODELS ---
		// OBS.: `glUseProgram()` picks which shaders run for every draw call after it.
		glUseProgram(modelProgram);
		
		// Lights: set once per frame, shared by every model drawn below.
		glUniform3fv(mdl::MODEL_AMBIENT_LOCATION    , 1, &scn::AMBIENT_LIGHT[0]   );
		glUniform3fv(mdl::MODEL_LIGHT_POS_LOCATION  , 1, &scene.navi.position[0]  );  // Navi carries the light.
		glUniform3fv(mdl::MODEL_LIGHT_COLOR_LOCATION, 1, &scn::NAVI_LIGHT_COLOR[0]);
		
		// Static models: M was computed once, in `scn::arrangeScene()`.
		for ( int id = 0; id < scn::MODEL_COUNT; id++ ) {
			for ( const glm::mat4& M : scene.statics[id] ) {
				mdl::drawModel(models[id], M);
			}
		}
		// Dynamic models: M is computed now, from whatever the keyboard made of the transform.
		mdl::drawModel(models[scn::MODEL_NAVI ], xform::modelMatrix(scene.navi ));
		mdl::drawModel(models[scn::MODEL_HEART], xform::modelMatrix(scene.heart));
		
		
		// --- DRAW SKY BOX ---
		// Drawn last: wherever a model already stands, the depth test discards the sky's (farther) fragments,
		// saving the work of shading them.
		glUseProgram(skyboxProgram);
		sky::drawSkybox(sky);
		
		
		// --- DISPLAY FRAME ---
		// Show the finished frame (back buffer becomes front buffer).
		glfwSwapBuffers(window);
	}
	
	
	// █▀ █ █ █ █ ▀█▀ █▀▄ █▀█ █ █ █ █▄ █
	// ▄█ █▀█ █▄█  █  █▄▀ █▄█ ▀▄▀▄▀ █ ▀█
	// SHUTDOWN                         
	
	sky::deleteSkybox(sky);
	for ( mdl::Model& model : models ) {
		mdl::deleteModel(model);
	}
	
	glDeleteBuffers(1, &cameraBuffer);
	
	glDeleteProgram(skyboxProgram);
	glDeleteProgram(modelProgram);
	
	glfwTerminate();
	
	return EXIT_SUCCESS;
}
