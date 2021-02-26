#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "render.h"
#include "world.h"
#include "expose.h"

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

// TODO REMOVE
#include "collision.h"

int main(int argc, char *argv[]) {
	goto a;
	bbox_t boxes[27];
	int numBoxes = 0;

	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 3; x++) {
			boxes[numBoxes] = (bbox_t){(v3d){x, y, 0}, (v3d){1, 1, 1}};
			numBoxes++;
		}
	}

	boxes[numBoxes] = (bbox_t){(v3d){0, 0, 1}, (v3d){1, 1, 1}};
	numBoxes++;

	bbox_t eBox = {(v3d){.5, .5, .99}, (v3d){1, 1, 1}};

	v3d result = collideResolveMultiple(eBox, boxes, numBoxes);

	printf("final resolution: ");
	v3d_print(result);
	printf("\nfinal position: ");
	v3d_print(v3d_add(eBox.offset, result));
	printf("\n");

	return 0;
a:

	init();
	loadMedia();

	v3i dims = {2, 2, 1};
	world_t *world = createWorld(dims);
	generateWorld(world);

	unsigned int lastTime, thisTime = SDL_GetTicks();

	bool quit = false;
	SDL_Event e;
	const Uint8 *kbState = SDL_GetKeyboardState(NULL);

	v3i moveInputs;
	const int SPEED = 3; // TODO move this somewhere better idk where
	const v3d moveDown = {SPEED, SPEED, 0};
	const v3d moveRight = {SPEED, -SPEED, 0};

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

		// movement
		moveInputs = (v3i){0, 0, 0};
		if (kbState[SDL_SCANCODE_W])
			moveInputs.y--;
		if (kbState[SDL_SCANCODE_S])
			moveInputs.y++;
		if (kbState[SDL_SCANCODE_A])
			moveInputs.x--;
		if (kbState[SDL_SCANCODE_D])
			moveInputs.x++;
		if (kbState[SDL_SCANCODE_LSHIFT])
			moveInputs.z--;
		if (kbState[SDL_SCANCODE_LCTRL])
			moveInputs.z++;
		
		world->player->move = v3d_add(
			v3d_scale(moveRight, moveInputs.x),
			v3d_scale(moveDown, moveInputs.y)
		);
		world->player->move.z = moveInputs.z * 2;

		// tick
		thisTime = SDL_GetTicks();
		tickWorld(world, thisTime - lastTime);
		updateCamera(world);
		lastTime = thisTime;

		v3d_print(world->player->pos);
		printf("\n");

		// gfx
		exposeWorld(world);
		SDL_RenderClear(renderer);
		renderWorld(world);
		SDL_RenderPresent(renderer);

		// so my laptop doesn't explode
		SDL_Delay(5);
	}

	destroyWorld(world);
	world = NULL;
	onClose();
	
	return 0;
}
