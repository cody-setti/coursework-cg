proj1.o: proj1.cpp
	g++ proj1.cpp glad/src/gl.c \
    -Iglad/include \
    -lglfw -lGL -ldl \
    -o main