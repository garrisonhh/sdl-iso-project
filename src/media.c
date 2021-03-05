#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "render.h"
#include "textures.h"
#include "sprites.h"

void media_init() {
	int img_flags = IMG_INIT_PNG;
	if (!(IMG_Init(img_flags) & img_flags)) {
		printf("SDL_image could not initialize:\n%s\n", IMG_GetError());
		exit(1);
	}

	textures_init();
	textures_load();
	sprites_load();
}

void media_quit() {
	textures_destroy();
	sprites_destroy();
	IMG_Quit();
}

SDL_Texture *load_sdl_texture(char *path) {
	SDL_Texture *new_texture = NULL;
	SDL_Surface *loaded_surface = IMG_Load(path);
	if (loaded_surface == NULL) {
		printf("unable to load image %s:\n%s\n", path, IMG_GetError());
		exit(1);
	}

	new_texture = SDL_CreateTextureFromSurface(renderer, loaded_surface);
	if (new_texture == NULL) {
		printf("unable to create texture from %s:\n%s\n", path, SDL_GetError());
		exit(1);
	}

	SDL_FreeSurface(loaded_surface);
	return new_texture;
}
