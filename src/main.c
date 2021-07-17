#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "app.h"
#include "game.h"
#include "lib/vector.h"
#include <ghh/utils.h>
#include "render.h"
#include "render/gui.h"
#include "render/fonts.h"
#include "block/blocks.h"
#include "textures.h"
#include "sprites.h"

SDL_Window *WINDOW = NULL;

#ifdef _WIN32
// I hate programming C on windows.
#define MAIN WinMain
#else
#define MAIN main
#endif

void init(void);
void quit_all(void);

int MAIN(int argc, char **argv) {
	init();

	app_run();

	quit_all();

	return 0;
}

void init() {
	vector_check_structs();

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize:\n%s\n", SDL_GetError());
		exit(1);
	}

	WINDOW = SDL_CreateWindow("sdl-iso-project",
							  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
							  SCREEN_WIDTH, SCREEN_HEIGHT,
							  SDL_WINDOW_SHOWN);

	if (WINDOW == NULL) {
		printf("WINDOW could not be created:\n%s\n", SDL_GetError());
		exit(1);
	}

	int img_flags = IMG_INIT_PNG;
	if (!(IMG_Init(img_flags) & img_flags)) {
		printf("SDL_image could not initialize:\n%s\n", IMG_GetError());
		exit(1);
	}

	render_init(WINDOW);
	gui_init();
	game_init();

	fonts_load();
	textures_load();
	sprites_load();
	blocks_load();

	gui_load();

	app_menu_init();

	// makes sure the first frame (loading screen) renders properly
	SDL_RenderPresent(RENDERER);
}

void quit_all() {
	app_menu_quit();

	textures_destroy();
	sprites_destroy();
	blocks_destroy();

	render_quit();
	SDL_DestroyWindow(WINDOW);
	WINDOW = NULL;

	IMG_Quit();
	SDL_Quit();
}

void load_screen() {
	// draw loading text
	v2i loading_pos = {0, 0};
	SDL_SetRenderDrawColor(RENDERER, 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderClear(RENDERER);
	font_render(FONT_UI, "loading...", loading_pos);
	SDL_RenderPresent(RENDERER);
}
