#include <SDL2/SDL.h>
#include "world.h"

#ifndef RENDER_H
#define RENDER_H

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 800
#define VOXEL_WIDTH 32
#define VOXEL_HEIGHT 32
#define NUM_TEXTURES 5

typedef struct {
	SDL_Texture* top;
	SDL_Texture* side; // use SDL_RenderCopyEx to render flipped
} vox_tex;

extern SDL_Renderer *renderer;

void initRenderer(SDL_Window *);
void destroyRenderer(void);
void loadMedia(void);
void destroyMedia(void);
void renderWorld(world_t *);

#endif
