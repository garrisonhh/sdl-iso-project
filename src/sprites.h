#ifndef SPRITES_H
#define SPRITES_H

#include <SDL2/SDL.h>
#include <json-c/json.h>
#include "vector.h"

struct sprite_t {
	SDL_Texture *texture;
	// x, y are relative to entity_t position
	v2i pos, size;
};
typedef struct sprite_t sprite_t;

extern sprite_t **sprites;

void sprites_load(json_object *);
void sprites_destroy(void);
size_t sprite_index(char *);
void render_sprite(sprite_t *, v2i);

#endif
