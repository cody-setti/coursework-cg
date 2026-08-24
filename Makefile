proj1: proj1.cpp
	g++ proj1.cpp glad/src/gl.c -Iglad/include -lglfw -lGL -ldl -o main

.PHONY: clean run

clean:
	rm -f main

run:
	./main
