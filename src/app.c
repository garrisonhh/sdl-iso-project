#include <SDL2/SDL.h>
#include <stdbool.h>
#include "app.h"
#include "game.h"
#include "render.h"
#include "render/fonts.h"

app_state_e APP_STATE = APP_MAIN_MENU;

void app_main_menu(void);

void app_run() {
	while (APP_STATE != APP_EXIT) {
		switch (APP_STATE) {
		case APP_MAIN_MENU:
			app_main_menu();
			break;
		case APP_GAME:
			game_main();
			break;
		case APP_EXIT:
			break;
		}
	}
}

void app_main_menu() {
	bool QUIT = false;
	SDL_Event event;

	const int num_options = 2;
	const char *options[] = {
		"new world",
		"exit"
	};
	v2i pos;
	int choice = 0;

	const int line_height = font_line_height(FONT_MENU) + 4;
	const v2i char_size = font_char_size(FONT_MENU);

	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);

	while (!QUIT) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				QUIT = true;
				APP_STATE = APP_EXIT;
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
				case SDLK_ESCAPE:
					QUIT = true;
					APP_STATE = APP_EXIT;
					break;
				case SDLK_w:
				case SDLK_UP:
					choice = (choice - 1 + num_options) % num_options;
					break;
				case SDLK_s:
				case SDLK_DOWN:
					choice = (choice + 1) % num_options;
					break;
				case SDLK_RETURN:
					switch (choice) {
					case 0:
						QUIT = true;
						APP_STATE = APP_GAME;
						break;
					case 1:
						QUIT = true;
						APP_STATE = APP_EXIT;
						break;
						}
						break;
					}
					break;
			}
		}

		SDL_RenderClear(renderer);

		// TODO render static text for this
		pos = char_size;
		font_render_text(FONT_MENU, "untitled", pos);

		pos	= (v2i){3 * char_size.x, 3 * line_height};

		for (int i = 0; i < num_options; ++i) {
			font_render_text(FONT_MENU, options[i], pos);
			pos.y += line_height;
		}

		pos.x = char_size.x;
		pos.y = (3 + choice) * line_height;

		font_render_text(FONT_MENU, ">", pos);

		SDL_RenderPresent(renderer);
	}
}
