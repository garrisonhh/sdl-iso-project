#include <SDL2/SDL.h>
#include "world.h"

#ifndef RENDER_H
#define RENDER_H

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 800

typedef enum {
	TEX_TEXTURE,
	TEX_VOXELTEXTURE
} block_tex_type;   

typedef struct {
	SDL_Texture* top;
	SDL_Texture* side; // use SDL_RenderCopyEx to render flipped
} vox_tex;

typedef struct {
	block_tex_type type;
	SDL_Texture *texture;
	vox_tex *voxelTexture;
	bool transparent; // TODO
} texture_t;

extern SDL_Renderer *renderer;

void initRenderer(SDL_Window *);
void destroyRenderer(void);
void loadMedia(void);
void destroyMedia(void);
void renderWorld(world_t *);

#endif
