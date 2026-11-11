# Falling moon

```sh
make run
```

- Run from the **project root**: shader and asset paths are relative to it.
- Needs GLFW installed on the system. GLAD, GLM and stb_image are vendored in `external/`.
- The binary goes to `build/main`.

## Data flow

```
 disk          src/loading/       src/scene/          src/                  
 ────────────  ─────────────────  ──────────────────  ───────────           
 .obj .mtl ──► wavefront.hpp ──┐                                            
                               ├─►  model.hpp ────┐                         
 .png ───────► texture.hpp ────┘                  │                         
                          └──────► skybox.hpp ────┤                         
                                                  │                         
 .vert .frag ► shader.hpp ────────────────────────┼─► main.cpp              
                                                  │                         
                                   camera.hpp ────┘                         
                                       ▲                                    
                                       └───────────── input.hpp ◄── keyboard
                                                           ▲ 
                                  constants.hpp ───────────┤
                                    scene.hpp   ───────────┘
```

## Commands

### Camera controls
- <kbd>w</kbd> walk forwards
- <kbd>s</kbd> walk backwards
- <kbd>a</kbd> walk to the left
- <kbd>d</kbd> walk to the right
- <kbd>Space</kbd> go up
- <kbd>ctrl</kbd> go down
- <kbd>shift</kbd> sprint

### Navi controls
- <kbd>up</kbd> moves Navi away from the camera
- <kbd>down</kbd> moves Navi towards the camera
- <kbd>right</kbd> moves Navi to the right of the camera
- <kbd>left</kbd> moves Navi to the left of the camera
