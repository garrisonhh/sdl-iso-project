#include <SDL2/SDL.h>
#include <stdbool.h>
#include "app.h"
#include "game.h"
#include "menu.h"
#include "render.h"
#include "render/fonts.h"
#include "data_structures/array.h"

enum menu_state_e {
	MENU_MAIN = 0,
};
typedef enum menu_state_e menu_state_e;

app_state_e APP_STATE = APP_MENU;
menu_state_e MENU_STATE;

// corresponds to menu_state_e
#define NUM_MENUS 1
menu_screen_t *MENUS[NUM_MENUS];

bool MENU_QUIT;

void app_menu(void);

void app_run() {
	while (APP_STATE != APP_EXIT) {
		switch (APP_STATE) {
		case APP_MENU:
			app_menu();
			break;
		case APP_GAME:
			game_main();
			break;
		case APP_EXIT:
			break;
		}
	}
}

// menu stuff
void app_menu_exit() {
	APP_STATE = APP_EXIT;
	MENU_QUIT = true;
}

void app_menu_new_world() {
	APP_STATE = APP_GAME;
	MENU_QUIT = true;
}

void app_menu_init() {
	v2i pos;
	int line_h = font_line_height(FONT_MENU);

	for (int i = 0; i < NUM_MENUS; ++i)
		MENUS[i] = menu_screen_create();

	// main menu
	pos = (v2i){20, 20};
	menu_screen_add_text_button(MENUS[MENU_MAIN], "untitled", pos, NULL);

	pos.y += line_h * 2;
	menu_screen_add_text_button(MENUS[MENU_MAIN], "new world", pos, app_menu_new_world);

	pos.y += line_h;
	menu_screen_add_text_button(MENUS[MENU_MAIN], "exit", pos, app_menu_exit);
}

void app_menu_quit() {
	for (int i = 0; i < NUM_MENUS; ++i)
		menu_screen_destroy(MENUS[i]);
}

void app_menu() {
	MENU_STATE = MENU_MAIN;

	// menu loop
	MENU_QUIT = false;
	SDL_Event event;

	while (!MENU_QUIT) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				app_menu_exit();
				break;
			case SDL_MOUSEBUTTONUP:
				v2i pos = {event.button.x, event.button.y};
				menu_screen_click(MENUS[MENU_STATE], pos);
				break;
			}
		}

		SDL_SetRenderDrawColor(renderer, 0x0F, 0x2F, 0x3F, 0xFF);
		SDL_RenderClear(renderer);

		menu_screen_tick(MENUS[MENU_STATE]);

		menu_screen_render(MENUS[MENU_STATE]);

		SDL_RenderPresent(renderer);
	}
}
