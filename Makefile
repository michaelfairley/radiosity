default: build

build:
	mkdir -p out
	g++ -Wall -pedantic -Werror radiosity.cpp -o out/main `sdl2-config --libs --cflags` -framework OpenGL -isystem include

run: build
	./out/main
