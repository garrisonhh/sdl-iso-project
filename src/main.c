#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "app.h"
#include "game.h"
#include "vector.h"
#include "render.h"
#include "render/gui.h"
#include "render/fonts.h"
#include "block_gen.h"
#include "textures.h"
#include "sprites.h"

SDL_Window *WINDOW = NULL;

void init(void);
void quit_all(void);

int main(int argc, char *argv[]) {
	init();

	app_run();

	quit_all();
	
	return 0;
}

void init() {
	vector_check_structs();

	// sdl
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
	block_gen_load();

	gui_load();

	// draw loading text
	SDL_RenderPresent(renderer);
	v2i loading_pos = {0, 0};
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderClear(renderer);
	font_render_text(FONT_UI, "loading...", loading_pos);
	SDL_RenderPresent(renderer);
}

void quit_all() {
	textures_destroy();
	sprites_destroy();
	block_gen_destroy();

	render_quit();
	SDL_DestroyWindow(WINDOW);
	WINDOW = NULL;

	IMG_Quit();
	SDL_Quit();
}
