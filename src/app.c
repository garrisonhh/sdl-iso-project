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
		"start",
		"exit"
	};
	v2i pos;
	int choice = 0;

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
				case SDLK_w:
					choice = (choice - 1 + num_options) % num_options;
					break;
				case SDLK_s:
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
		pos = (v2i){0, 0};
		fonts_render_text(FONT_UI, "untitled", pos);

		pos	= (v2i){
			2 * FONTS[FONT_UI].char_size.x,
			2 * FONTS[FONT_UI].char_size.y
		};

		for (int i = 0; i < num_options; ++i) {
			fonts_render_text(FONT_UI, options[i], pos);
			pos.y += FONTS[FONT_UI].char_size.y;
		}

		pos.x = 0;
		pos.y = (2 + choice) * FONTS[FONT_UI].char_size.y;

		fonts_render_text(FONT_UI, ">", pos);

		SDL_RenderPresent(renderer);
	}
}
