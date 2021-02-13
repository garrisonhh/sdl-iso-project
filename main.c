#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "render.h"
#include "textures.h"
#include "vector.h"
#include "world.h"
#include "expose.h"

/*
in this file: main() and initialization functions
*/

SDL_Window *window = NULL;
void init() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize:\n%s\n", SDL_GetError());
		exit(1);
	}
	window = SDL_CreateWindow("render this bitch",
							  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT,
							  SDL_WINDOW_SHOWN);
	if (window == NULL) {
		printf("window could not be created:\n%s\n", SDL_GetError());
		exit(1);
	}
	
	initRenderer(window);
	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags)) {
		printf("SDL_image could not initialize:\n%s\n", IMG_GetError());
		exit(1);
	}
}

// naming this "close" results in seg fault lmao. func name conflict in sdl somewhere?
void onClose() {
	destroyMedia();
	destroyRenderer();

	SDL_DestroyWindow(window);
	window = NULL;

	IMG_Quit();
	SDL_Quit();
}

int main() {
	init();
	loadMedia();

	vector3 dims = {2, 2, 1};
	world_t *world = createWorld(dims);
	generateWorld(world);

	/*unsigned int lastTime, frameTick = 100;
	unsigned int thisTime = SDL_GetTicks();
	float frameRate;
	unsigned int frames[100];*/

	bool quit = false;
	SDL_Event e;

	while (!quit) {
		while (SDL_PollEvent(&e) != 0) {
			switch (e.type) {
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_KEYDOWN:
				switch (e.key.keysym.sym) { // nothing wrong with this. very normal code.
				case SDLK_ESCAPE:
					quit = true;
					break;
				}
				break;
			}
		}

		exposeWorld(world);
		SDL_RenderClear(renderer);
		renderWorld(world);
		SDL_RenderPresent(renderer);

		// TODO render FPS on screen
		/*lastTime = thisTime;
		thisTime = SDL_GetTicks();
		for (int i = 100; i > 0; i--) {
			frames[i] = frames[i - 1];
		}
		frames[0] = thisTime - lastTime;
		frameTick--;
		if (frameTick == 0) {
			frameRate = 0;
			for (int i = 0; i < 100; i++) {
				frameRate += frames[i];
			}
			frameRate /= 100;
			printf("FPS: %.2f\n", 1000 / frameRate);
			frameTick = 100;
		}*/

		SDL_Delay(5); // so my laptop doesn't explode
	}

	destroyWorld(world);
	world = NULL;
	onClose();
	return 0;
}
