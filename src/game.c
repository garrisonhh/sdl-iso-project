#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_mutex.h>
#include <stdio.h>
#include "game.h"
#include "app.h"
#include "render.h"
#include "render/gui.h"
#include "player.h"
#include "camera.h"
#include "lib/mytimer.h"

SDL_mutex *RENDER_INFO_LOCK, *LAST_INFO_LOCK;
SDL_sem *MAIN_DONE = NULL, *GAME_LOOP_DONE = NULL;
SDL_sem *EVENTS_PUMPED = NULL;
render_info_t *RENDER_INFO = NULL, *LAST_INFO = NULL;

int NEW_WORLD_SIZE = 0;
world_gen_type_e WORLD_TYPE = 0;

bool QUIT;

void game_init() {
	RENDER_INFO_LOCK = SDL_CreateMutex();
	LAST_INFO_LOCK = SDL_CreateMutex();

	render_game_init();
}

int game_loop(void *arg) {
	world_t *world = world_create(NEW_WORLD_SIZE, WORLD_TYPE);

	player_init(world);

	size_t i, num_packets;
	int num_events = 0, max_events = 64;
	SDL_Event events[max_events];
	render_info_t *next_render_info;
	mytimer_t *timer = mytimer_create(60);

	while (!QUIT) {
		// event handling
		SDL_SemWait(EVENTS_PUMPED);
		num_events = SDL_PeepEvents(events, max_events, SDL_GETEVENT,
									SDL_FIRSTEVENT, SDL_LASTEVENT);

		for (i = 0; i < num_events; ++i) {
			switch (events[i].type) {
			case SDL_QUIT:
				QUIT = true;
				APP_STATE = APP_EXIT;
				break;
			case SDL_KEYDOWN:
				switch (events[i].key.keysym.sym) {
				case SDLK_ESCAPE:
					QUIT = true;
					APP_STATE = APP_MENU;
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
					camera_scale(events[i].wheel.y > 0);
					break;
			case SDL_MOUSEBUTTONDOWN:
				if (events[i].button.button == SDL_BUTTON_LEFT) {
					v2i pos = {events[i].button.x, events[i].button.y};
					player_click(world, pos);
				}
				break;
			}
		}

		// ticking
		mytimer_tick(timer);
		player_tick();
		world_tick(world, mytimer_get_tick(timer));

		// update render info + gui
		camera_set_pos(player_get_pos());
		next_render_info = render_gen_info(world);

		num_packets = next_render_info->bg_packets->size;

		if (next_render_info->fg_packets != NULL)
			num_packets += next_render_info->fg_packets->size;

		// send render info to main thread
		SDL_SemWait(MAIN_DONE);

		GUI_DATA.fps = mytimer_get_fps(timer);
		GUI_DATA.packets = num_packets;
		gui_tick();

		SDL_LockMutex(RENDER_INFO_LOCK);
		RENDER_INFO = next_render_info;
		SDL_UnlockMutex(RENDER_INFO_LOCK);

		SDL_SemPost(GAME_LOOP_DONE);

		// free old info instance
		if (LAST_INFO != NULL) {
			SDL_LockMutex(LAST_INFO_LOCK);
			render_info_destroy(LAST_INFO);
			LAST_INFO = NULL;
			SDL_UnlockMutex(LAST_INFO_LOCK);
		}
	}

	if (RENDER_INFO != NULL) {
		render_info_destroy(RENDER_INFO);
		RENDER_INFO = NULL;
	}

	if (LAST_INFO != NULL) {
		render_info_destroy(LAST_INFO);
		LAST_INFO = NULL;
	}

	world_destroy(world);
	mytimer_destroy(timer);

	return 0;
}

void game_main() {
	MAIN_DONE = SDL_CreateSemaphore(1);
	GAME_LOOP_DONE = SDL_CreateSemaphore(0);
	EVENTS_PUMPED = SDL_CreateSemaphore(0);

	SDL_Thread *game_loop_thread = SDL_CreateThread(game_loop, "game_loop", NULL);

	if (game_loop_thread == NULL) {
		printf("spawning game loop thread failed.\n");
		exit(1);
	}

	QUIT = false;

	while (!QUIT) {
		// events
		SDL_PumpEvents();
		SDL_SemPost(EVENTS_PUMPED);

		// rendering
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
		SDL_RenderPresent(RENDERER);
		SDL_SemPost(MAIN_DONE);
	}

	SDL_WaitThread(game_loop_thread, NULL);

	SDL_DestroySemaphore(MAIN_DONE);
	SDL_DestroySemaphore(GAME_LOOP_DONE);
	SDL_DestroySemaphore(EVENTS_PUMPED);
}
