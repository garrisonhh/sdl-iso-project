#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "media.h"
#include "render.h"
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
							  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT,
							  SDL_WINDOW_SHOWN);

	if (window == NULL) {
		printf("window could not be created:\n%s\n", SDL_GetError());
		exit(1);
	}
	
	render_init(window);	
	media_init();
}

// naming this "close" results in seg fault lmao. func name conflict in sdl somewhere?
void on_close() {
	media_quit();
	render_destroy();

	SDL_DestroyWindow(window);
	window = NULL;

	SDL_Quit();
}

int main(int argc, char *argv[]) {
	init();

	// world
	world_t *world = world_create(3);
	world_generate(world);

	// framerate capping
	unsigned int last_time = SDL_GetTicks(), this_time;
	/*
	unsigned int target_framerate = 120, frame_start;
	int frame_delay;
	unsigned int target_frame_time = 1000 / target_framerate;
	*/

	// controls
	SDL_Event e;
	const uint8_t *kb_state = SDL_GetKeyboardState(NULL);
	v3i move_inputs;
	const v3d move_down = {PLAYER_SPEED, PLAYER_SPEED, 0};
	const v3d move_right = {PLAYER_SPEED, -PLAYER_SPEED, 0};
	v3d move;
	bool jump = false;

	// loop
	bool quit = false;

	while (!quit) {
		//frame_start = SDL_GetTicks();

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
					camera_change_scale(e.wheel.y < 0);
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

		if (jump && world->player->on_ground)
			world->player->ray.dir.z += 9.0;

		// tick
		this_time = SDL_GetTicks();
		world_tick(world, this_time - last_time);
		camera_update(world);
		last_time = this_time;

		// gfx
		render_clear_screen();
		render_world(world);
		SDL_RenderPresent(renderer);

		// limit framerate to `target_framerate`
		/*
		if ((frame_delay = frame_start + target_frame_time - SDL_GetTicks()) > 0)
			SDL_Delay(frame_delay);
		*/
	}

	world_destroy(world);
	on_close();
	
	return 0;
 }
