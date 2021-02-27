#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "media.h"
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
	
	render_init(window);
	int img_flags = IMG_INIT_PNG;
	if (!(IMG_Init(img_flags) & img_flags)) {
		printf("SDL_image could not initialize:\n%s\n", IMG_GetError());
		exit(1);
	}
}

// naming this "close" results in seg fault lmao. func name conflict in sdl somewhere?
void on_close() {
	media_destroy();
	render_destroy();

	SDL_DestroyWindow(window);
	window = NULL;

	IMG_Quit();
	SDL_Quit();
}

#include "collision.h" // TODO REMOVE

int main(int argc, char *argv[]) {
	/*
	// generate boxes
	bbox_t boxes[50];
	int num_boxes = 0;
	int x, y, z;

	for (x = 0; x < 3; x++) {
		for (y = 0; y < 3; y++) {
			for (z = 0; z < 1; z++) {
				boxes[num_boxes++] = (bbox_t){(v3d){x, y, z}, (v3d){1, 1, 1}};
			}
		}
	}

	// entity_tick test
	entity_t entity;
	int ms = 10;

	entity.ray = (ray_t){(v3d){1, 1, 1}, (v3d){1, 1, -.5}};
	// entity.size = (v3d){1, 1, 1};

	entity_tick(&entity, ms, boxes, num_boxes);
	
	return 0;
	// TODO REMOVE ^^^
	*/
	init();
	media_load();

	v3i dims = {2, 2, 1};
	world_t *world = world_create(dims);
	world_generate(world);

	unsigned int last_time, this_time = SDL_GetTicks();

	bool quit = false;
	SDL_Event e;
	const Uint8 *kb_state = SDL_GetKeyboardState(NULL);

	v3i move_inputs;
	const int SPEED = 3; // TODO move this somewhere better idk where
	const v3d move_down = {SPEED, SPEED, 0};
	const v3d move_right = {SPEED, -SPEED, 0};

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
		move_inputs = (v3i){0, 0, 0};
		if (kb_state[SDL_SCANCODE_W])
			move_inputs.y--;
		if (kb_state[SDL_SCANCODE_S])
			move_inputs.y++;
		if (kb_state[SDL_SCANCODE_A])
			move_inputs.x--;
		if (kb_state[SDL_SCANCODE_D])
			move_inputs.x++;
		if (kb_state[SDL_SCANCODE_LSHIFT])
			move_inputs.z--;
		if (kb_state[SDL_SCANCODE_LCTRL])
			move_inputs.z++;
		
		world->player->ray.dir = v3d_add(
			v3d_scale(move_right, move_inputs.x),
			v3d_scale(move_down, move_inputs.y)
		);
		world->player->ray.dir.z = move_inputs.z * 2;

		// tick
		this_time = SDL_GetTicks();
		world_tick(world, this_time - last_time);
		update_camera(world);
		last_time = this_time;

		// gfx
		expose_world(world);
		SDL_RenderClear(renderer);
		render_world(world);
		SDL_RenderPresent(renderer);

		// so my laptop doesn't explode
		SDL_Delay(20);
	}

	world_destroy(world);
	world = NULL;
	on_close();
	
	return 0;
}
