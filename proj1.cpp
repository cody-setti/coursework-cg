#include <array>
#include <cmath>
#include <glad/gl.h>                                  // GLAD = Multi-Language GL/GLES/EGL/GLX/WGL Loader-Generator.
							// 	> GLES = OpenGL for Embedded Systems
							// 	> EGL  = Embedded-System Graphics Library
							// 	> GLX  = OpenGL Extension to the X Window System
							// 	> WGL  = (MS) Windows Graphics Library
							// Load OS-specific OpenGL function pointers at runtime.

#include <GLFW/glfw3.h>                                 // GLFW = Graphics Library FRAMEWORK.
							// Create an OS window and capture keyboard inputs.

#include <glm/glm.hpp>                                  // GLM = OpenGL Mathematics.
							// Provide GLSL-compatible vector and matrix types.
							// 	> GLSL = OpenGL Shading Language

                 					// GTC = GLM Template Core.
#include <glm/gtc/matrix_transform.hpp>			// For mathematical transformations like rotation and scaling.
#include <glm/gtc/type_ptr.hpp>                         // Convert C++ GLM matrix objects into flat memory arrays for OpenGL.

#include <iostream>                                     // iostream = Input/Output Stream
#include <vector>
							// Output compilation errors to the terminal.


std::array<float, 3> sphere_to_cartesian(float u, float v, float r)
{
    float x = r * std::sin(v) * std::cos(u);
    float y = r * std::sin(v) * std::sin(u);
    float z = r * std::cos(v);

    return {x, y, z};
}

std::vector<float> sphere_vertex(float r, int num_sectors, int num_stacks) {
    float sector_step = (2 * M_PI) / num_sectors;
    float stack_step = M_PI / num_stacks;
    std::vector<float> vertex_list;

    for(int i = 0; i < num_sectors; i++) {
        for(int j =0; j < num_stacks; j++) {
            float u = i * sector_step;
            float v = j * stack_step;

            float un = 0.0;
            if(i+1 == num_sectors) un = 2 * M_PI;
            else un = (i+1) * sector_step;

            float vn = 0.0;
            if(j+1 == num_stacks) vn = M_PI;
            else vn = (j+1) * stack_step;

            std::array<float, 3> p0 = sphere_to_cartesian(u, v, r);
            std::array<float, 3> p1 = sphere_to_cartesian(u, vn, r);
            std::array<float, 3> p2 = sphere_to_cartesian(un, v, r);
            std::array<float, 3> p3 = sphere_to_cartesian(un, vn, r);
            
            vertex_list.push_back(p0[0]); vertex_list.push_back(p0[1]); vertex_list.push_back(p0[2]);
            vertex_list.push_back(p2[0]); vertex_list.push_back(p2[1]); vertex_list.push_back(p2[2]);
            vertex_list.push_back(p1[0]); vertex_list.push_back(p1[1]); vertex_list.push_back(p1[2]);

            vertex_list.push_back(p3[0]); vertex_list.push_back(p3[1]); vertex_list.push_back(p3[2]);
            vertex_list.push_back(p1[0]); vertex_list.push_back(p1[1]); vertex_list.push_back(p1[2]);
            vertex_list.push_back(p2[0]); vertex_list.push_back(p2[1]); vertex_list.push_back(p2[2]);
        }
    }

    return vertex_list;
}





bool wireframeMode = false;                             // Track the current state of the polygon rendering mode.
							// Wireframe mode draws only the edges ("wires") of the polygon.

void key_callback(
		GLFWwindow* window,	// Specific OS window that receive the input event.
		int key,		// Standardized GLFW key token (e.g., GLFW_KEY_P).
		int scancode,		// The physical location of the key on the board, ignoring language layouts.
		int action,		// The state change (e.g., GLFW_PRESS, GLFW_RELEASE, etc).
		int mods		// A bit field of held modifier keys (Shift, Ctrl, Alt, etc.)
) {							// Required signature by GLFW to intercept OS keyboard events.

    if (key == GLFW_KEY_P && action == GLFW_PRESS) {    // Isolate the specific 'p' key press event.
        wireframeMode = !wireframeMode;
        if (wireframeMode)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  // Instruct the rasterizer to only draw lines between vertices.
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  // Instruct the rasterizer to fill the polygons with color.
    }
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

int main() {                                            // Main execution entry point for a C++ application.
    glfwInit();                                         // Initialize the GLFW library before any of its functions can be used.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);      // Request OpenGL version 3.x to ensure modern pipeline features.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);      // Request OpenGL version x.3 (creating a 3.3 context).
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Explicitly disable deprecated legacy functions.

    GLFWwindow* window = glfwCreateWindow(800, 800, "Proj 1", NULL, NULL); // Allocate memory and ask the OS for a window.
    glfwMakeContextCurrent(window);                     // Tell OpenGL that subsequent API calls apply to this specific window.
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

    std::vector vertices = sphere_vertex(0.3, 32, 32);


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

    while (!glfwWindowShouldClose(window)) {            // Keep the application running until an exit signal is received.
        glClearColor(0.6f, 0.3f, 0.0f, 1.0f);           // Define the default background color for the window.
        glClear(GL_COLOR_BUFFER_BIT);                   // Wipe the previous frame's pixels, preventing visual trailing.

        glUseProgram(shaderProgram);                    // Instruct the GPU to use our specific custom shader pipeline.

        glm::mat4 trans = glm::mat4(1.0f);              // Initialize an identity matrix (doing nothing by default).
        //trans = glm::rotate(trans, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f)); // Required to apply a rotation transformation over time.
        
        unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform"); // Find where the matrix variable lives in GPU memory.
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans)); // Send the matrix data from CPU to GPU before drawing.

        glBindVertexArray(VAO);                         // Recall all the data mapping configurations we set up earlier.
        glUniform4f(loc_color, 1.0, 1.0, 0.0, 1.0);
        glDrawArrays(GL_TRIANGLES, 0, vertices.size()/3);               // Execute the actual rasterization command, drawing 3 vertices.


        glfwSwapBuffers(window);                        // Push the newly drawn frame to the screen (double buffering).
        glfwPollEvents();                               // Process pending OS events like mouse movements or 'p' key presses.
    }                                                   // Syntax to close the render loop.

    glfwTerminate();                                    // Cleanly destroy windows, cursors, and free OS resources.
    return 0;                                           // Standard C++ exit code indicating successful execution.
}                                                       // Syntax to close the main function.
