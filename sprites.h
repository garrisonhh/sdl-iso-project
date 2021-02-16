#include <SDL2/SDL.h>

#ifndef SPRITE_H
#define SPRITE_H

typedef struct {
	SDL_Texture *texture;
	// offsets are relative to entity_t position
	int offsetX;
	int offsetY;
	int w;
	int h;
} sprite_t;

extern sprite_t **sprites;

void loadSprites(void);
void destroySprites(void);

#endif
