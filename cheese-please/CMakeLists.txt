# --- PROJECT SETUP ---
cmake_minimum_required(VERSION 3.20)      # Refuse to run on older CMake, and adopt its modern
                                          # policy set instead of legacy-compatible behaviour.

project(projeto1                          # The project's name. Free-form; it labels the build.
	LANGUAGES C CXX                       # Probe a C compiler *and* a C++ one: GLAD ships as C
)                                         # (gl.c), our own code is C++.

# --- COMPILER SETTINGS ---
set(CMAKE_CXX_STANDARD 17)                # Becomes -std=c++17 under gcc/clang, /std:c++17 under MSVC.
set(CMAKE_CXX_STANDARD_REQUIRED ON)       # Hard-fail instead of silently falling back to an older standard.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)     # Emit build/compile_commands.json, which clangd and most
                                          # editors read to resolve <glad/gl.h> for autocompletion.

if(NOT CMAKE_BUILD_TYPE)                  # Nobody said which kind of build they wanted, so pick the
	set(CMAKE_BUILD_TYPE Release)         # optimised one rather than CMake's unoptimised default.
endif()

# --- LOCATE SYSTEM DEPENDENCIES ---
# Each of these asks the machine where the library lives rather than hardcoding a path,
# which is what lets this same file build on Arch, Ubuntu, macOS or Windows.
find_package(OpenGL REQUIRED)             # The driver's GL library itself.  -> target OpenGL::GL
find_package(glfw3 3.3 REQUIRED)          # Window creation and keyboard input. -> target glfw
find_package(glm REQUIRED)                # GLSL-compatible vector/matrix math.  -> target glm::glm
                                          # REQUIRED = stop now with a readable message, rather
                                          # than a cryptic linker error a minute from now.

# --- BUILD GLAD ---
# GLAD is not an installed library: it is C source code generated once from the OpenGL spec
# and committed into this repository on purpose. That is why building needs no Python,
# no code generation step and no network access.
add_library(glad STATIC                   # Compile it into libglad.a.
	external/glad/src/gl.c
)
target_include_directories(glad PUBLIC    # PUBLIC = glad needs this path to compile itself, AND
	external/glad/include                 # so does anything that links glad. That is how the
)                                         # executable below silently inherits <glad/gl.h>.

# --- BUILD THE PROGRAM ---
add_executable(projeto1                   # The binary lands at build/projeto1.
	src/main.cpp                          # the window, the GPU and the render loop
	src/geometry.cpp                      # what a vertex and a mesh are, and the primitives
	src/scene.cpp                         # the state, the keyboard, one matrix per object
	src/objects/table.cpp                 # ... and one file per object of the scene:
	src/objects/rat.cpp
	src/objects/mousetrap.cpp
	src/objects/cheese.cpp
	src/objects/wine_glass.cpp
)                                         # (transform.hpp and palette.hpp are header-only.)

target_include_directories(projeto1 PRIVATE
	src                                   # So that every file spells its includes the same
)                                         # way -- "geometry.hpp", "objects/objects.hpp" --
                                          # no matter which directory it sits in itself.

target_compile_options(projeto1 PRIVATE   # Ask for the warnings that catch real mistakes. A clean
	-Wall -Wextra                         # build is only meaningful if the compiler was told to talk.
)

target_link_libraries(projeto1 PRIVATE    # PRIVATE = nothing links `projeto1`, so it has no
	glad                                  # requirements to pass further up the chain.
	glfw                                  # Every name here drags in its own include paths,
	glm::glm                              # compile definitions and link flags automatically.
	OpenGL::GL
)
