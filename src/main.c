#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "content.h"
#include "render.h"
#include "camera.h"
#include "world.h"
#include "player.h"
#include "utils.h"

SDL_Window *window = NULL;

void init() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize:\n%s\n", SDL_GetError());
		exit(1);
	}

	window = SDL_CreateWindow("sdl-iso-project",
							  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
							  SCREEN_WIDTH, SCREEN_HEIGHT,
							  SDL_WINDOW_SHOWN);

	if (window == NULL) {
		printf("window could not be created:\n%s\n", SDL_GetError());
		exit(1);
	}
	
	int img_flags = IMG_INIT_PNG;
	if (!(IMG_Init(img_flags) & img_flags)) {
		printf("SDL_image could not initialize:\n%s\n", IMG_GetError());
		exit(1);
	}
	
	render_init(window);	
	content_init();
}

void quit_all() {
	content_quit();
	render_destroy();

	SDL_DestroyWindow(window);
	window = NULL;

	IMG_Quit();
	SDL_Quit();
}

#include "pathing.h" // TODO REMOVE

int main(int argc, char *argv[]) {
	init();

	// world
	world_t *world = world_create(1);
	world_generate(world);

	/* pathing test
	v3i start = {0, 0, 1};
	v3i end = {31, 31, 1};

	timeit_start();
	list_t *path = path_find(world->path_net, start, end);	
	timeit_end("path found in");

	v3i *cur;
	
	while (path->size > 0) {
		cur = list_pop(path);
		v3i_print(NULL, *cur);
	}

	exit(0);
	*/

	// time
	unsigned int last_time = SDL_GetTicks(), this_time;

	// controls
	SDL_Event e;
	const uint8_t *kb_state = SDL_GetKeyboardState(NULL);
	v3i move_inputs;
	const v3d move_down = {1, 1, 0};
	const v3d move_right = {1, -1, 0};
	v3d move;
	bool jump = false;

	// loop
	bool quit = false;

	while (!quit) {
		while (SDL_PollEvent(&e)) {
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
				case SDL_KEYUP:
					switch (e.key.keysym.sym) {
						case SDLK_SPACE:
							jump = false;
							break;
					}
					break;
				case SDL_MOUSEWHEEL:
					camera_change_scale(e.wheel.y > 0);
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

		if (move_inputs.x || move_inputs.y)
			move = v3d_scale(move, PLAYER_SPEED / fabs(v3d_magnitude(move)));

		world->player->ray.dir.x = move.x;
		world->player->ray.dir.y = move.y;

		if (jump && world->player->on_ground)
			world->player->ray.dir.z += 9.0;

		// tick
		this_time = SDL_GetTicks();
		world_tick(world, this_time - last_time);
		camera_update(world);
		last_time = this_time;

		// gfx
		render_world(world);
		SDL_RenderPresent(renderer);
	}

	world_destroy(world);
	quit_all();
	
	return 0;
 }
