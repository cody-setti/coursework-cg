build/main: src/main.cpp src/*.hpp src/*/*.hpp external/glad/src/gl.c
	mkdir -p build
	g++ src/main.cpp external/glad/src/gl.c -Isrc -Iexternal/glad/include -Iexternal/glm -Iexternal/stb -lglfw -o build/main

run       : build/main
	./build/main

.PHONY: run
