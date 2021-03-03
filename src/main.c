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
#include "utils.h"

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

int main(int argc, char *argv[]) {
	init();
	media_load();

	v3i dims = {4, 4, 2};
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
	v3d move;
	bool jump = false;

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
						case SDLK_SPACE:
							jump = true;
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

		move = v3d_add(
			v3d_scale(move_right, move_inputs.x),
			v3d_scale(move_down, move_inputs.y)
		);

		world->player->ray.dir.x = move.x;
		world->player->ray.dir.y = move.y;

		if (jump) {
			world->player->ray.dir.z += 9.0;
			jump = false;
		}

		// tick
		this_time = SDL_GetTicks();
		world_tick(world, this_time - last_time);
		update_camera(world);
		last_time = this_time;

		// gfx
		expose_world(world);
		render_clear_screen();
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
