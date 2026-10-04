# Spooky House

An OpenGL 4.6 scene: textured `.obj` models (grass, pine, oak, hut) inside a sky cube.

## Build & run

```sh
make run
```

- Run from the **project root**: shader and asset paths are relative to it.
- Needs GLFW installed on the system. GLAD, GLM and stb_image are vendored in `external/`.
- The binary goes to `build/main`.

## Layout

```
assets/               one folder per model (.obj + .mtl + images), plus skybox/      
shaders/              textured.{vert,frag} for models, skybox.{vert,frag} for the sky
src/                                                                                 
├── main.cpp          window + context, load everything, render loop, shutdown       
├── input.hpp         every key that acts on the scene (camera included)             
│                                                                                    
├── loading/          files on disk → C++ data / GL objects                          
│   │                                                                                
│   ├── wavefront.hpp       .obj/.mtl   → plain C++ data                             
│   ├── texture.hpp         images      → GL texture (2D and cube-map)               
│   └── shader.hpp          .vert/.frag → GL program                                 
│                                                                                    
└── scene/            things in the world                                            
    │                                                                                
    ├── model.hpp         textured model: load, draw, delete                         
    ├── skybox.hpp        sky cube      : load, draw, delete                         
    └── camera.hpp        camera state + View matrix                                 
```

`scene/` includes `loading/`, never the other way around. Everything is header-only and compiled as one unit from `main.cpp`.

## What's left to do

| Requirement                                                                                                                                             | Where                                                               |
|---------------------------------------------------------------------------------------------------------------------------------------------------------|---------------------------------------------------------------------|
| Objective: explore the scene by manipulating the camera                                                                                                 | key reading in `updateInput()`; move/turn functions in `camera.hpp` |
| 7: scale, rotation and translation must be applied, each to a different model, each triggered independently from the keyboard                           | `updateInput()` in `input.hpp`                                      |
| 9: exploration must stay within the "edge" of the sky and the terrain. The camera may pass freely through models, just not through the sky or the floor | limits in `camera.hpp`                                              |
| 10: the user must be able to see the polygon mesh at will: pressing `p` shows the objects' meshes if hidden, or hides them if shown                     | `keyCallback()` in `input.hpp`                                      |

Key bindings all go in `input.hpp` so clashes are easy to spot. Rules about the camera itself (like the requirement 9 limits) go in `camera.hpp`.

Two things you'll run into:

- **`keyCallback()` can't take extra arguments** (GLFW fixes its signature).
  To flip a wireframe flag from it, use `glfwSetWindowUserPointer()` / `glfwGetWindowUserPointer()` or a global.
- **The model transforms have no state yet.** Each M is built inline in the render loop (`main.cpp`, the `M = ...` blocks) and the models currently spin on a timer (`glfwGetTime()`).
  For keys to move a model, its position / rotation / scale has to live somewhere `updateInput()` can change and the render loop can read, e.g. a small struct passed in alongside `camera`.
