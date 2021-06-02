#include <SDL2/SDL.h>
#include <stdbool.h>
#include "app.h"
#include "game.h"
#include "world.h"
#include "menu.h"
#include "render.h"
#include "render/fonts.h"
#include "lib/array.h"
#include "lib/utils.h"

enum menu_state_e {
	MENU_MAIN,
	MENU_NEW_WORLD,

	NUM_MENUS
};
typedef enum menu_state_e menu_state_e;

app_state_e APP_STATE = APP_MENU;
menu_state_e MENU_STATE;

menu_t *MENUS[NUM_MENUS];

bool MENU_QUIT;

char *WORLD_SIZE_TEXT;
char *WORLD_TYPE_TEXT;

const char *WORLD_TYPE_NAMES[NUM_WORLD_TYPES] = {"flat", "alien"};

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

// main menu buttons
void app_menu_exit() {
	APP_STATE = APP_EXIT;
	MENU_QUIT = true;
}

void app_menu_new_world() {
	MENU_STATE = MENU_NEW_WORLD;
}

// new world buttons
void app_menu_check_world_size() {
	sprintf(WORLD_SIZE_TEXT, "%i (%i)", NEW_WORLD_SIZE, 16 << NEW_WORLD_SIZE);
}

void app_menu_inc_world_size() {
	NEW_WORLD_SIZE = MIN(NEW_WORLD_SIZE + 1, 5);
	app_menu_check_world_size();
}

void app_menu_dec_world_size() {
	NEW_WORLD_SIZE = MAX(NEW_WORLD_SIZE - 1, 0);
	app_menu_check_world_size();
}

void app_menu_generate_world() {
	APP_STATE = APP_GAME;
	MENU_QUIT = true;
}

void app_menu_check_world_type() {
	strcpy(WORLD_TYPE_TEXT, WORLD_TYPE_NAMES[WORLD_TYPE]);
}

void app_menu_next_world_type() {
	WORLD_TYPE = (WORLD_TYPE + 1) % NUM_WORLD_TYPES;
	app_menu_check_world_type();
}

void app_menu_init() {
	v2i pos, aligned = {30, 20};
	int line_h = font_line_height(FONT_MENU);
	int char_w = font_char_size(FONT_MENU).x;
	menu_t *menu;

	for (int i = 0; i < NUM_MENUS; ++i)
		MENUS[i] = menu_create();

	WORLD_SIZE_TEXT = malloc(sizeof(char) * 30);
	WORLD_TYPE_TEXT = malloc(sizeof(char) * 30);
	app_menu_check_world_size();
	app_menu_check_world_type();

	/*
	 * main menu
	 */
	menu = MENUS[MENU_MAIN];

	pos = aligned;
	menu_add_text_label(menu, ">>> untitled <<<", pos);

	pos.y += line_h * 2;
	menu_add_text_button(menu, "new world", pos, app_menu_new_world);

	pos.y += line_h;
	menu_add_text_button(menu, "exit", pos, app_menu_exit);
	
	/*
	 * new world
	 */
	menu = MENUS[MENU_NEW_WORLD];

	pos = aligned;
	menu_add_text_label(menu, ">>> new world <<<", pos);

	// choose size
	pos.y += line_h * 2;
	menu_add_text_label(menu, "size:", pos);

	pos.x += char_w * 8;
	menu_add_dynamic_text(menu, &WORLD_SIZE_TEXT, pos);

	pos.x += char_w * 8;
	menu_add_text_button(menu, "<", pos, app_menu_dec_world_size);

	pos.x += char_w * 2;
	menu_add_text_button(menu, ">", pos, app_menu_inc_world_size);

	// choose world type
	pos.x = aligned.x;
	pos.y += line_h;
	menu_add_text_label(menu, "type:", pos);

	pos.x += char_w * 8;
	menu_add_dynamic_text(menu, &WORLD_TYPE_TEXT, pos);

	pos.x += char_w * 8;
	menu_add_text_button(menu, ">", pos, app_menu_next_world_type);

	// generate
	pos.x = aligned.x;
	pos.y += line_h * 2;
	menu_add_text_button(menu, "generate", pos, app_menu_generate_world);
}

void app_menu_quit() {
	for (int i = 0; i < NUM_MENUS; ++i)
		menu_destroy(MENUS[i]);

	free(WORLD_SIZE_TEXT);
}

void app_menu() {
	SDL_Rect fill_rect = {
		20, 0,
		(SCREEN_WIDTH >> 2) + 20, SCREEN_HEIGHT
	};

	// menu loop
	MENU_STATE = MENU_MAIN;
	MENU_QUIT = false;
	SDL_Event event;

	while (!MENU_QUIT) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				app_menu_exit();
				break;
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE)
					app_menu_exit();
				break;
			case SDL_MOUSEBUTTONUP:
				v2i pos = {event.button.x, event.button.y};
				menu_click(MENUS[MENU_STATE], pos);
				break;
			}
		}

		SDL_SetRenderDrawColor(renderer, 0x0F, 0x2F, 0x3F, 0xFF);
		SDL_RenderClear(renderer);

		SDL_SetRenderDrawColor(renderer, 0x1F, 0x1F, 0x1F, 0x7F);
		SDL_RenderFillRect(renderer, &fill_rect);

		menu_tick(MENUS[MENU_STATE]);
		menu_render(MENUS[MENU_STATE]);

		SDL_RenderPresent(renderer);
	}
}
