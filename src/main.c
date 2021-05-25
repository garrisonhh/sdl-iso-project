#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_mutex.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "render.h"
#include "render/gui.h"
#include "render/fonts.h"
#include "camera.h"
#include "world.h"
#include "player.h"
#include "mytimer.h"
#include "utils.h"
#include "textures.h"
#include "block_gen.h"
#include "vector.h"

SDL_Window *window = NULL;

bool QUIT = false;

SDL_sem *MAIN_DONE, *RENDER_DONE;
SDL_mutex *RENDER_INFO_LOCK;
render_info_t *RENDER_INFO, *LAST_INFO;

void init() {
	vector_check_structs();

	// sdl
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

	// threading
	MAIN_DONE = SDL_CreateSemaphore(0);
	RENDER_DONE = SDL_CreateSemaphore(1);
	RENDER_INFO_LOCK = SDL_CreateMutex();

	// init game stuff
	render_init(window);
	gui_init();

	fonts_load();
	textures_load();
	block_gen_load();

	gui_load();

	// draw loading text
	v2i loading_pos = {0, 0};
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderClear(renderer);
	fonts_render_text(FONT_UI, "loading...", loading_pos);
	// calls twice for double buffer
	SDL_RenderPresent(renderer);
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
		LAST_INFO = RENDER_INFO;
		RENDER_INFO = NULL;

		SDL_UnlockMutex(RENDER_INFO_LOCK);

		gui_render();
		SDL_RenderPresent(renderer);

		SDL_SemPost(RENDER_DONE);
	}

	return 0;
}

#include <time.h>
#include "data_structures/hashmap.h"

int main(int argc, char *argv[]) {
	init();

	// init game stuff
	world_t *world = world_create(1);

	world_generate(world);

	player_init(world);
	camera_set_block_size(world->block_size);

	// game loop vars
	size_t i;

	SDL_Event event;
	SDL_Thread *render_thread = SDL_CreateThread(render, "Rendering", NULL);

	if (render_thread == NULL) {
		printf("spawning render thread failed.\n");
		exit(1);
	}

	render_info_t *next_render_info;
	int packets;

	mytimer_t *timer = mytimer_create(256);

	while (!QUIT) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					QUIT = true;
					break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym) {
						case SDLK_ESCAPE:
							QUIT = true;
							break;
						case SDLK_e:
							camera_rotate(true);
							break;
						case SDLK_q:
							camera_rotate(false);
							break;
						case SDLK_F1:
							gui_toggle_debug();
							break;
						case SDLK_F2:
							player_toggle_godmode();
							break;

					}
					break;
				case SDL_MOUSEWHEEL:
					camera_change_scale(event.wheel.y > 0);
					break;
			}
		}

		mytimer_tick(timer);

		player_tick();
		world_tick(world, mytimer_get_tick(timer));

		// update render info + gui
		camera_set_pos(player_get_pos());
		next_render_info = render_gen_info(world);

		packets = 0;
		for (i = 0; i < next_render_info->z_levels; ++i)
			packets += next_render_info->packets[i]->size;

		SDL_SemWait(RENDER_DONE);

		gui_update(mytimer_get_fps(timer), packets, world);

		SDL_LockMutex(RENDER_INFO_LOCK);

		RENDER_INFO = next_render_info;

		SDL_UnlockMutex(RENDER_INFO_LOCK);
		SDL_SemPost(MAIN_DONE);

		if (LAST_INFO != NULL)
			render_info_destroy(LAST_INFO);
	}

	SDL_WaitThread(render_thread, NULL);

	world_destroy(world);
	quit_all();
	
	return 0;
 }
