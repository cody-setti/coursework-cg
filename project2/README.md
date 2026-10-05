# Spooky House

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
```

## What's left to do
**5.** "Os modelos devem ser ajustados convenientemente e de um jeito coerente com o mundo real. Exemplos: sofás e pessoas não flutuam; um óculos não é maior que uma mesa, etc."

**7.** "As transformações escala, rotação e translação devem ser aplicadas, cada uma em um modelo diferente e com acionamento via teclado de forma independente."

**9.** "O seu programa deve restringir a exploração do cenário dentro dos limites da "borda" do céu e terreno. Não é necessário restringir o acesso aos modelos, ou seja, a câmera pode passar livremente "por dentro" dos modelos (com exceção do céu e piso)."

**10.** "O usuário deve poder visualizar a malha poligonal quando quiser. Caso ele aperte `p`, as malhas dos objetos devem ser exibidas (caso estejam ocultas) ou ocultadas (caso estejam sendo exibidas)."
