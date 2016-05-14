default: build

build:
	mkdir -p out
	g++ radiosity.cpp -o out/main `sdl2-config --libs --cflags` -framework OpenGL -Iinclude

run: build
	./out/main
