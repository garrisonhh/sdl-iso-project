#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_mutex.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "render/render.h"
#include "render/gui.h"
#include "render/fonts.h"
#include "camera.h"
#include "world.h"
#include "player.h"
#include "utils.h"
#include "textures.h"
#include "block_gen.h"

SDL_Window *window = NULL;

bool QUIT = false;

// threaded stuff
SDL_sem *MAIN_DONE, *RENDER_DONE;
render_info_t *RENDER_INFO;
SDL_mutex *RENDER_INFO_LOCK;

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

	MAIN_DONE = SDL_CreateSemaphore(0);
	RENDER_DONE = SDL_CreateSemaphore(1);
	RENDER_INFO_LOCK = SDL_CreateMutex();

	render_init(window);
	gui_init();

	fonts_load();
	textures_load();
	block_gen_load();

	gui_load();

	SDL_RenderPresent(renderer);

	// draw loading text
	v2i loading_pos = {0, 0};
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderClear(renderer);
	fonts_render_text(FONT_UI, "loading...", loading_pos);
	SDL_RenderPresent(renderer);
}

void quit_all() {
	textures_destroy();
	block_gen_destroy();

	render_quit();

	SDL_DestroyWindow(window);
	window = NULL;

	IMG_Quit();
	SDL_Quit();
}

int render(void *arg) {
	while (!QUIT) {
		SDL_SemWait(MAIN_DONE);

		SDL_LockMutex(RENDER_INFO_LOCK);
		render_from_info(RENDER_INFO);
		SDL_UnlockMutex(RENDER_INFO_LOCK);

		gui_render();
		SDL_RenderPresent(renderer);

		SDL_SemPost(RENDER_DONE);
	}

	return 0;
}

int main(int argc, char *argv[]) {
	init();

	// world
	world_t *world = world_create(3);
	world_generate(world);

	camera_set_block_size(world->block_size);

	// time
	double last_tick = timeit_get_time(), this_tick = 0;
	const int num_ticks = 128;
	double ticks[num_ticks];
	double tick_avg;
	int tick_idx = 0;
	int i;

	for (i = 0; i < 32; ++i)
		ticks[i] = 0;

	// controls TODO move control code to player.c
	SDL_Event event;
	const uint8_t *kb_state = SDL_GetKeyboardState(NULL);
	v3i move_inputs;
	const v3d move_down = {1, 1, 0};
	const v3d move_right = {1, -1, 0};
	v3d move;
	bool jump = false;

	// threading
	SDL_Thread *render_thread = SDL_CreateThread(render, "Rendering", NULL);

	if (render_thread == NULL) {
		printf("thread failed.\n");
		exit(1);
	}

	render_info_t *next_render_info;

	while (!QUIT) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					QUIT = true;
					break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym) { // nothing wrong with this. very normal code.
						case SDLK_ESCAPE:
							QUIT = true;
							break;
						case SDLK_BACKQUOTE:
							gui_toggle_debug();
							break;
						case SDLK_SPACE:
							jump = true;
							break;
						case SDLK_e:
							camera_rotate(true);
							break;
						case SDLK_q:
							camera_rotate(false);
							break;
					}

					break;
				case SDL_KEYUP:
					switch (event.key.keysym.sym) {
						case SDLK_SPACE:
							jump = false;
							break;
					}

					break;
				case SDL_MOUSEWHEEL:
					camera_change_scale(event.wheel.y > 0);
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

		move = camera_reverse_rotated_v3d(move);

		world->player->ray.dir.x = move.x;
		world->player->ray.dir.y = move.y;

		if (jump && world->player->on_ground)
			world->player->ray.dir.z += 9.0;

		// tick
		last_tick = this_tick;
		this_tick = timeit_get_time();

		world_tick(world, this_tick - last_tick);
		camera_set_pos(world->player->ray.pos);

		ticks[tick_idx++] = this_tick - last_tick;
		tick_idx %= num_ticks;
		tick_avg = 0;

		for (i = 0; i < 32; ++i)
			tick_avg += ticks[i];

		gui_update(1.0 / (tick_avg / 32), world);

		// update info
		next_render_info = render_gen_info(world);

		SDL_SemWait(RENDER_DONE);

		SDL_LockMutex(RENDER_INFO_LOCK);
		RENDER_INFO = next_render_info;
		SDL_UnlockMutex(RENDER_INFO_LOCK);

		SDL_SemPost(MAIN_DONE);
	}

	SDL_WaitThread(render_thread, NULL);

	world_destroy(world);
	quit_all();
	
	return 0;
 }
