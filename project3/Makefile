build/main: src/main.cpp $(wildcard src/*.hpp src/*/*.hpp) external/glad/src/gl.c external/stb/stb_image.c
	mkdir -p build
	g++ src/main.cpp external/glad/src/gl.c external/stb/stb_image.c -Isrc -Iexternal/glad/include -Iexternal/glm -Iexternal/stb -lglfw -o build/main

run       : build/main
	./build/main

.PHONY: run
