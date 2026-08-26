#include <cstdlib>
#include <glad/gl.h>                                  // GLAD = Multi-Language GL/GLES/EGL/GLX/WGL Loader-Generator.
							// 	> GLES = OpenGL for Embedded Systems
							// 	> EGL  = Embedded-System Graphics Library
							// 	> GLX  = OpenGL Extension to the X Window System
							// 	> WGL  = (MS) Windows Graphics Library
							// Load OS-specific OpenGL function pointers at runtime.

#include <GLFW/glfw3.h>                                 // GLFW = Graphics Library FRAMEWORK.
							// Create an OS window and capture keyboard inputs.

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>                                  // GLM = OpenGL Mathematics.
							// Provide GLSL-compatible vector and matrix types.
							// 	> GLSL = OpenGL Shading Language

                 					// GTC = GLM Template Core.
#include <glm/gtc/matrix_transform.hpp>			// For mathematical transformations like rotation and scaling.
#include <glm/gtc/type_ptr.hpp>                         // Convert C++ GLM matrix objects into flat memory arrays for OpenGL.

#include <iostream>                                     // iostream = Input/Output Stream
							// Output compilation errors to the terminal.
#include <vector>
#include <array>
#include <cmath>

#include "rat.cpp"
#include "aux_funcs.cpp"


typedef struct State { //struct that allows us to not need global variables to use in key_callback().
    float speed_x_rat, speed_y_rat, inc_x_rat, inc_y_rat;
    int rat_flip;
    float ang_speed_mousetrap, inc_mousetrap;
    float speed_scale_cheese, inc_cheese;
    bool wireframeMode;
} State;

State* init_state(float inc_x_rat, float inc_y_rat, float inc_mousetrap, float inc_cheese) {
    State *aux = (State*)malloc(sizeof(State));
    aux->speed_x_rat = 0.0;
    aux->speed_y_rat = 0.0;
    aux->ang_speed_mousetrap = 0.0;
    aux-> speed_scale_cheese = 0.0;
    aux->wireframeMode = false;
    aux->rat_flip = 0;
    aux->inc_cheese = inc_cheese;
    aux->inc_mousetrap = inc_mousetrap;
    aux->inc_x_rat = inc_x_rat;
    aux->inc_y_rat = inc_y_rat;
    return aux;
}


                            //https://discourse.glfw.org/t/passing-parameters-to-callbacks/848
void key_callback(
		GLFWwindow* window,	// Specific OS window that receive the input event.
		int key,		// Standardized GLFW key token (e.g., GLFW_KEY_P).
		int scancode,		// The physical location of the key on the board, ignoring language layouts.
		int action,		// The state change (e.g., GLFW_PRESS, GLFW_RELEASE, etc).
		int mods		// A bit field of held modifier keys (Shift, Ctrl, Alt, etc.)
) {							// Required signature by GLFW to intercept OS keyboard events.
    State* state = static_cast<State*>(glfwGetWindowUserPointer(window));
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {    // Isolate the specific 'p' key press event.
        state->wireframeMode = !state->wireframeMode;
        if (state->wireframeMode)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  // Instruct the rasterizer to only draw lines between vertices.
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  // Instruct the rasterizer to fill the polygons with color.
    }

    if(key == GLFW_KEY_LEFT) state->speed_x_rat -= state->inc_x_rat;
    if(key == GLFW_KEY_RIGHT) state->speed_x_rat += state->inc_x_rat;
    if(key == GLFW_KEY_DOWN) state->speed_y_rat -= state->inc_y_rat;
    if(key == GLFW_KEY_UP) state->speed_y_rat += state->inc_y_rat;
    if(key == GLFW_KEY_F) state->rat_flip += 1;
}

const char* vertexShaderSource =
    "#version 330 core\n"  // Specify the modern OpenGL 3.3 profile.
    "layout (location = 0) in vec3 Pos;;\n"                              // Declare the vertex positions attribute
    "uniform mat4 transform;\n"                         // Required to receive the single geometric transformation matrix from the CPU.
    "void main() {\n"                                   // Entry point for the vertex shader program.
    "   gl_Position = transform * vec4(Pos, 1.0);\n" // Calculate final vertex position by applying the matrix.
    "}\0";                                              // Null-terminator for standard C-string parsing.

const char* fragmentShaderSource =
    "#version 330 core\n"// Specify the modern OpenGL 3.3 profile matching the vertex shader.
    "uniform vec4 color;\n"                             // The color of the polygon
    "void main() {\n"                                   // Entry point for the fragment shader program.
    "   gl_FragColor = color;\n"                         // Assign the RGB color
    "}\n\0";                                            // Null-terminator for standard C-string parsing.




//====================================================================================
//----------------------------------draw functions------------------------------------
//====================================================================================
//Keep in mind that we must transpose the matrix first before plugging it into a glm::mat4


void draw_rat(
        size_t ini_pos, 
        size_t num, 
        int loc_transform, 
        int loc_color, 
        float& pos_x, 
        float& pos_y, 
        bool& flip_done, 
        float& flip_angle, 
        State *state
) {
        //The matrixes we're going to use
        glm::mat4 rotation_z;
        glm::mat4 rotation_x;
        glm::mat4 rotation_y;
        glm::mat4 rot_final;

        if(fabs(state->speed_x_rat) <= state->inc_x_rat * 0.9) state->speed_x_rat = 0.0; //zeroes the speed if is smaller than an increment
        if(fabs(state->speed_y_rat) <= state->inc_y_rat * 0.9) state->speed_y_rat = 0.0; //zeroes the speed if is smaller than an increment
        if(state->rat_flip == 0) { //if the rat is not ready to flip
            pos_x += state->speed_x_rat; //updates the position
            pos_y += state->speed_y_rat; //updates the position
        }
        else if(state->rat_flip >= 1){
            if(pos_y <= 0.5 && !flip_done) pos_y += state->rat_flip * state->inc_y_rat;
            else if(flip_done) pos_y -= state->rat_flip * state->inc_y_rat;
            else flip_angle -= state->rat_flip * 0.04;
        }
        if(flip_angle <= - M_PI * 2) {
            flip_angle = 0.0;
            flip_done = true;
        }
        if(flip_done && pos_y <= 0.0) {
            state->rat_flip -= 1;
            flip_done = false;
        }
        
        if(pos_y < 0.0) pos_y = 0.0; //Limits the position of the rat. below 0.0 there's the table


        if(pos_y > 0.0 && state->rat_flip == 0) { //The mouse is crawling on the walls
            rotation_x = glm::mat4( //This rotates the mouse 90 degrees counter-clockwise, so it's top is facing us
                1.0, 0.0, 0.0, 0.0,
                0.0, 0.0, -1.0, 0.0,
                0.0, 1.0, 0.0, 0.0,
                0.0, 0.0, 0.0, 1.0
            );
            if(state->speed_x_rat == 0.0 && state->speed_y_rat == 0.0) rot_final = rotation_x; //if the rat is stationary, there's no speed vector
            else { //otherwise, we have a speed vector defined and we can rotate the rat so it's facing that direction
                float hip = sqrt(state->speed_x_rat * state->speed_x_rat + state->speed_y_rat * state->speed_y_rat);
                float cos = -state->speed_x_rat / hip; // minus sign because the rat's face originally points to the right
                float sin = -state->speed_y_rat / hip;

                rotation_z = glm::mat4(
                    cos, sin, 0.0, 0.0,
                    -sin, cos, 0.0, 0.0,
                    0.0, 0.0, 1.0, 0.0,
                    0.0, 0.0, 0.0, 1.0
                );
                rot_final = rotation_z * rotation_x;
            }
        }

        else { //if the rat is on the table (the sky is blue) or fliping
            float cos_t = cos(flip_angle);
            float sin_t = sin(flip_angle);

            rotation_x = glm::mat4( //This rotates the mouse at the flip angle
                1.0, 0.0, 0.0, 0.0,
                0.0, cos_t, sin_t, 0.0,
                0.0, -sin_t, cos_t, 0.0,
                0.0, 0.0, 0.0, 1.0
            );

            if(state->speed_x_rat > 0.0) //facing right
            {
                rotation_y = glm::mat4(
                    -1.0, 0.0, 0.0, 0.0,
                    0.0, 1.0, 0.0, 0.0,
                    0.0, 0.0, -1.0, 0.0,
                    0.0, 0.0, 0.0, 1.0
                );
            }
            else { //facing left
                rotation_y = glm::mat4(
                    1.0, 0.0, 0.0, 0.0,
                    0.0, 1.0, 0.0, 0.0,
                    0.0, 0.0, 1.0, 0.0,
                    0.0, 0.0, 0.0, 1.0
                ); 
            }
            rot_final = rotation_x * rotation_y;
        } 

        //scale the rat
        glm::mat4 scale = glm::mat4(
            0.03, 0.0,  0.0,   0.0,
            0.0,  0.03, 0.0,   0.0,
            0.0,  0.0,  -0.03, 0.0, //TODO: discover why this needs to be negative
            0.0,  0.0,  0.0,   1.0
        );
        //moves the rat to the origin
        glm::mat4 translate_ori = glm::mat4(
            1.0,   0.0,  0.0, 0.0,
            0.0,   1.0,  0.0, 0.0,
            0.0,   0.0,  1.0, 0.0,
            -9.5, -3.0, -3.0, 1.0
        );
        //moves the rat through the screen
        glm::mat4 translate = glm::mat4(
            1.0,   0.0,  0.0, 0.0,
            0.0,   1.0,  0.0, 0.0,
            0.0,   0.0,  1.0, 0.0,
            pos_x, pos_y, 0.0, 1.0
        );
        glm::mat4 trans = translate * rot_final * scale * translate_ori;
        glUniformMatrix4fv(loc_transform, 1, GL_FALSE, glm::value_ptr(trans));
        glUniform4f(loc_color, 0.6, 0.3, 0.0, 1.0);
        glDrawArrays(GL_TRIANGLES, ini_pos/3, 192);

        glUniform4f(loc_color, 1.0, 0.9, 0.8, 1.0);
        glDrawArrays(GL_TRIANGLES, ini_pos/3 + 192, num/3 - 192);
}



//====================================================================================
//------------------------------------main--------------------------------------------
//====================================================================================

int main() {                                            // Main execution entry point for a C++ application.
    glfwInit();                                         // Initialize the GLFW library before any of its functions can be used.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);      // Request OpenGL version 3.x to ensure modern pipeline features.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);      // Request OpenGL version x.3 (creating a 3.3 context).
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Explicitly disable deprecated legacy functions.
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    GLFWwindow* window = glfwCreateWindow(800, 800, "Proj 1", NULL, NULL); // Allocate memory and ask the OS for a window.
    glfwMakeContextCurrent(window);                     // Tell OpenGL that subsequent API calls apply to this specific window.


    State *state = init_state(0.001, 0.001, 0.01, 0.001);
    glfwSetWindowUserPointer(window, (void*)state);
    glfwSetKeyCallback(window, key_callback);           // Register our custom input function with GLFW's event system.
    gladLoadGL((GLADloadfunc)glfwGetProcAddress); // Initialize GLAD using GLFW's memory pointers to map the OpenGL API.

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER); // Allocate GPU memory for a new vertex shader object.
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL); // Inject our GLSL string into the created shader object.
    glCompileShader(vertexShader);                      // Compile the GLSL string into GPU machine code.

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // Allocate GPU memory for a new fragment shader object.
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL); // Inject our GLSL string into the created shader object.
    glCompileShader(fragmentShader);                    // Compile the GLSL string into GPU machine code.

    unsigned int shaderProgram = glCreateProgram();     // Allocate GPU memory for the final linked shader pipeline.
    glAttachShader(shaderProgram, vertexShader);        // Link the compiled vertex logic to the final pipeline.
    glAttachShader(shaderProgram, fragmentShader);      // Link the compiled fragment logic to the final pipeline.
    glLinkProgram(shaderProgram);                       // Resolve inputs/outputs between the vertex and fragment shaders.
    glDeleteShader(vertexShader);                       // Free memory, as the individual shader is no longer needed after linking.
    glDeleteShader(fragmentShader);                     // Free memory, as the individual shader is no longer needed after linking.


    //====================================================================================
    //---------------------------vertex vector--------------------------------------------
    //====================================================================================
    std::vector<float> vertices;

    size_t ini_rat, num_rat;
    add_object(get_rat_vec(), vertices, ini_rat, num_rat);


    unsigned int VBO, VAO;                              // Declare handles for managing GPU memory objects.
    glGenVertexArrays(1, &VAO);                         // Generate an ID for our state-tracking object.
    glGenBuffers(1, &VBO);                              // Generate an ID for our raw data buffer.
    glBindVertexArray(VAO);                             // Start recording configuration states into this specific VAO.
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);                 // Activate the VBO so we can load data into it.
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW); // Actually copy the CPU array into GPU VRAM memory.

    int loc_vertex = glGetAttribLocation(shaderProgram, "Pos");
    glVertexAttribPointer(loc_vertex, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // Tell OpenGL how to extract the X,Y positions from the buffer.
    glEnableVertexAttribArray(loc_vertex);                       // Explicitly turn on the position attribute pipeline (location 0).

    int loc_color = glGetUniformLocation(shaderProgram, "color");


    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);



    float pos_x_rat = 0.5, pos_y_rat = 0.0, rat_flip_angle = 0.0;
    bool rat_flip_done = false;
    //====================================================================================
    //------------------------------------window loop-------------------------------------
    //====================================================================================

    while (!glfwWindowShouldClose(window)) {            // Keep the application running until an exit signal is received.


        glClearColor(0.0f, 1.0f, 1.0f, 1.0f);           // Define the default background color for the window.
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);                   // Wipe the previous frame's pixels, preventing visual trailing.

        glUseProgram(shaderProgram);                    // Instruct the GPU to use our specific custom shader pipeline.

        unsigned int loc_transform = glGetUniformLocation(shaderProgram, "transform"); // Find where the matrix variable lives in GPU memory.
        

        glBindVertexArray(VAO);                         // Recall all the data mapping configurations we set up earlier.

        draw_rat(ini_rat, num_rat, loc_transform, loc_color, pos_x_rat, pos_y_rat, rat_flip_done, rat_flip_angle, state);

        glfwSwapBuffers(window);                        // Push the newly drawn frame to the screen (double buffering).
        glfwPollEvents();                               // Process pending OS events like mouse movements or 'p' key presses.
    }                                                   // Syntax to close the render loop.

    glfwTerminate();                                    // Cleanly destroy windows, cursors, and free OS resources.
    return 0;                                           // Standard C++ exit code indicating successful execution.
}                                                       // Syntax to close the main function.
