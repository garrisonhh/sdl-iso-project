#ifndef SPRITES_H
#define SPRITES_H

#include <SDL2/SDL.h>
#include "vector.h"

typedef struct {
	SDL_Texture *texture;
	// x, y are relative to entity_t position
	v2i pos, size;
} sprite_t;

extern sprite_t **sprites;

void sprites_load(void);
void sprites_destroy(void);
void render_sprite(sprite_t *, v2i);

#endif
