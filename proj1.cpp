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
							// Output compilation errors to the terminal.



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
    "layout (location = 0) in vec2 aPos;\n"             // Map the incoming vertex position data to attribute location 0.
    "layout (location = 1) in vec3 aColor;\n"           // Map the incoming vertex color data to attribute location 1.
    "out vec3 ourColor;\n"                              // Declare an output variable to pass color data to the fragment shader.
    "uniform mat4 transform;\n"                         // Required to receive the single geometric transformation matrix from the CPU.
    "void main() {\n"                                   // Entry point for the vertex shader program.
    "   gl_Position = transform * vec4(aPos, 0.0, 1.0);\n" // Calculate final vertex position by applying the matrix.
    "   ourColor = aColor;\n"                           // Pass the specific vertex color down the pipeline.
    "}\0";                                              // Null-terminator for standard C-string parsing.

const char* fragmentShaderSource =
    "#version 330 core\n"// Specify the modern OpenGL 3.3 profile matching the vertex shader.
    "out vec4 FragColor;\n"                             // Define the final color output variable for the screen pixel.
    "in vec3 ourColor;\n"                               // Receive the interpolated color data from the vertex shader.
    "void main() {\n"                                   // Entry point for the fragment shader program.
    "   FragColor = vec4(ourColor, 1.0f);\n"            // Assign the RGB color and set full opacity (alpha = 1.0).
    "}\n\0";                                            // Null-terminator for standard C-string parsing.

int main() {                                            // Main execution entry point for a C++ application.
    glfwInit();                                         // Initialize the GLFW library before any of its functions can be used.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);      // Request OpenGL version 3.x to ensure modern pipeline features.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);      // Request OpenGL version x.3 (creating a 3.3 context).
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Explicitly disable deprecated legacy functions.

    GLFWwindow* window = glfwCreateWindow(800, 600, "Proj 1", NULL, NULL); // Allocate memory and ask the OS for a window.
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

    float vertices[] = {                                // Define the raw mathematical data for the composite shape.
         0.0f,  0.5f,  1.0f, 0.0f, 0.0f,                // Geometry and red color for the top vertex.
        -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,                // Geometry and green color for the bottom-left vertex.
         0.5f, -0.5f,  0.0f, 0.0f, 1.0f                 // Geometry and blue color for the bottom-right vertex.
    };                                                  // Syntax to close the array declaration.

    unsigned int VBO, VAO;                              // Declare handles for managing GPU memory objects.
    glGenVertexArrays(1, &VAO);                         // Generate an ID for our state-tracking object.
    glGenBuffers(1, &VBO);                              // Generate an ID for our raw data buffer.
    glBindVertexArray(VAO);                             // Start recording configuration states into this specific VAO.
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);                 // Activate the VBO so we can load data into it.
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // Actually copy the CPU array into GPU VRAM memory.

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0); // Tell OpenGL how to extract the X,Y positions from the buffer.
    glEnableVertexAttribArray(0);                       // Explicitly turn on the position attribute pipeline (location 0).

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float))); // Tell OpenGL how to extract the RGB colors.
    glEnableVertexAttribArray(1);                       // Explicitly turn on the color attribute pipeline (location 1).

    while (!glfwWindowShouldClose(window)) {            // Keep the application running until an exit signal is received.
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);           // Define the default background color for the window.
        glClear(GL_COLOR_BUFFER_BIT);                   // Wipe the previous frame's pixels, preventing visual trailing.

        glUseProgram(shaderProgram);                    // Instruct the GPU to use our specific custom shader pipeline.

        glm::mat4 trans = glm::mat4(1.0f);              // Initialize an identity matrix (doing nothing by default).
        trans = glm::rotate(trans, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f)); // Required to apply a rotation transformation over time.
        
        unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform"); // Find where the matrix variable lives in GPU memory.
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans)); // Send the matrix data from CPU to GPU before drawing.

        glBindVertexArray(VAO);                         // Recall all the data mapping configurations we set up earlier.
        glDrawArrays(GL_TRIANGLES, 0, 3);               // Execute the actual rasterization command, drawing 3 vertices.

        glfwSwapBuffers(window);                        // Push the newly drawn frame to the screen (double buffering).
        glfwPollEvents();                               // Process pending OS events like mouse movements or 'p' key presses.
    }                                                   // Syntax to close the render loop.

    glfwTerminate();                                    // Cleanly destroy windows, cursors, and free OS resources.
    return 0;                                           // Standard C++ exit code indicating successful execution.
}                                                       // Syntax to close the main function.
