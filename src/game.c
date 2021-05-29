#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_mutex.h>
#include "render.h"
#include "render/gui.h"
#include "world.h"
#include "player.h"
#include "camera.h"
#include "mytimer.h"

SDL_sem *MAIN_DONE, *GAME_LOOP_DONE;
SDL_mutex *RENDER_INFO_LOCK, *LAST_INFO_LOCK;
render_info_t *RENDER_INFO = NULL, *LAST_INFO = NULL;

bool QUIT = false;

int game_loop(void *arg) {

	world_t *world = world_create(1);
	world_generate(world);

	player_init(world);
	camera_set_block_size(world->block_size);

	SDL_Event event;
	render_info_t *next_render_info;
	mytimer_t *timer = mytimer_create(60);

	size_t num_packets;

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
					camera_scale(event.wheel.y > 0);
					break;
			}
		}

		mytimer_tick(timer);

		player_tick();
		world_tick(world, mytimer_get_tick(timer));

		// update render info + gui
		camera_set_pos(player_get_pos());
		next_render_info = render_gen_info(world);

		num_packets = next_render_info->bg_packets->size;

		if (next_render_info->fg_packets != NULL)
			num_packets += next_render_info->fg_packets->size;

		SDL_SemWait(MAIN_DONE);

		GUI_DATA.fps = mytimer_get_fps(timer);
		GUI_DATA.packets = num_packets;
		gui_tick();

		SDL_LockMutex(RENDER_INFO_LOCK);
		RENDER_INFO = next_render_info;
		SDL_UnlockMutex(RENDER_INFO_LOCK);

		SDL_SemPost(GAME_LOOP_DONE);

		if (LAST_INFO != NULL) {
			SDL_LockMutex(LAST_INFO_LOCK);
			render_info_destroy(LAST_INFO);
			LAST_INFO = NULL;
			SDL_UnlockMutex(LAST_INFO_LOCK);
		}
	}

	if (RENDER_INFO != NULL)
		render_info_destroy(RENDER_INFO);

	if (LAST_INFO != NULL)
		render_info_destroy(RENDER_INFO);

	world_destroy(world);
	mytimer_destroy(timer);

	return 0;
}

void game_main() {
	// threading
	MAIN_DONE = SDL_CreateSemaphore(1);
	GAME_LOOP_DONE = SDL_CreateSemaphore(0);
	RENDER_INFO_LOCK = SDL_CreateMutex();
	LAST_INFO_LOCK = SDL_CreateMutex();

	SDL_Thread *game_loop_thread = SDL_CreateThread(game_loop, "game_loop", NULL);

	if (game_loop_thread == NULL) {
		printf("spawning game loop thread failed.\n");
		exit(1);
	}

	while (!QUIT) {
		SDL_SemWait(GAME_LOOP_DONE);
		SDL_LockMutex(RENDER_INFO_LOCK);

		if (RENDER_INFO == NULL)
			break;

		render_from_info(RENDER_INFO);

		SDL_LockMutex(LAST_INFO_LOCK);
		LAST_INFO = RENDER_INFO;
		SDL_UnlockMutex(LAST_INFO_LOCK);

		RENDER_INFO = NULL;

		SDL_UnlockMutex(RENDER_INFO_LOCK);

		gui_render();
		SDL_RenderPresent(renderer);
		SDL_SemPost(MAIN_DONE);
	}

	SDL_WaitThread(game_loop_thread, NULL);
}
