#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "render.h"
#include "textures.h"
#include "sprites.h"

void media_load() {
	textures_load();
	sprites_load();
}

void media_destroy() {
	textures_destroy();
	sprites_destroy();
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
