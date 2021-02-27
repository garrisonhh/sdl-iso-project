#ifndef SPRITES_H
#define SPRITES_H

#include <SDL2/SDL.h>

typedef struct {
	SDL_Texture *texture;
	// x, y are relative to entity_t position
	int x, y, w, h;
} sprite_t;

extern sprite_t **sprites;

void sprites_load(void);
void sprites_destroy(void);

#endif
