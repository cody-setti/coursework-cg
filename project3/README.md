# Dark Times

```sh
make run
```

- Run from the **project root**: shader and asset paths are relative to it.
- Needs GLFW installed on the system. GLAD, GLM and stb_image are vendored in `external/`.
- The binary goes to `build/main`.

## Controls

| Keys            | Effect                          |
| --------------- | ------------------------------- |
| `1`             | Toggle ambient  light           |
| `2`             | Toggle Navi's   light           |
| `3`             | Toggle Candle's light           |
| `4`             | Toggle Heart's light            |
| `R` `F`         | Ambient  light dials            |
| `E` `Q`         | Diffuse  light dials            |
| `SPACE` `CTRL`  | Specular light dials            |
| `W` `A` `S` `D` | Fly                             |
| `LEFT SHIFT`    | Fly faster (hold)               |
| Cursor          | Look around                     |
| Scroll Wheel    | Zoom camera in/out              |
| Arrow keys      | Move Navi (translation)         |
| `P`             | Toggle the wireframe view       |
| `Esc`           | Quit                            |

## Data flow


```
 disk             .obj .mtl                .png           .vert .frag
. . . . . . . . . . . ▼ . . . . . . . . . .  ▼ . . . . . . . . ▼ . . 
 src/loading/    wavefront.hpp          texture.hpp        shader.hpp
                       │                  │  │                 │     
                       ├──────────────────┘  │                 │     
. . . . . . . . . . .  ▼ . . . . . . . . . . ▼ . . . . . . . . │ . . 
 src/render/       model.hpp             skybox.hpp            │     
                       │                     │                 │     
                       └─────────────────────┴────────────┐    │     
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . ▼ .  ▼ . . 
 src/                                                   MAIN.CPP     
. . . . . . . . . . . . . . . . . . . . . . . . . . . . ▲ .  ▲ . . . 
                 ┌─────────┬────────────────┬───────────┘    │       
                 │         │                │                │       
 src/world/      │  scene.hpp           camera.hpp           │       
                 │   ▲     ▲                ▲                │       
            transform.hpp  │                │                │       
                           │                │                │       
                           └────────────────┴───────────┐    │       
. . . . . . . . . . . . . . . . . . . . . . . . . . . . │ .  │ . . . 
 src/                                                  input.hpp     
. . . . . . . . . . . . . . . . . . . . . . . . . . . . .  ▲  . . . .
 devices                                        keys, cursor & scroll
```
