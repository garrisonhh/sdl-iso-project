#ifndef TEXTURES_H
#define TEXTURES_H

#include <SDL2/SDL.h>

typedef enum {
	TEX_TEXTURE = 0,
	TEX_VOXELTEXTURE = 1
} block_tex_type;   

typedef struct {
	SDL_Texture* top;
	SDL_Texture* side; // use SDL_RenderCopyEx to render flipped
} vox_tex;

typedef struct {
	block_tex_type type;
	SDL_Texture *texture;
	vox_tex *voxelTexture;
	bool transparent;
} texture_t;

extern texture_t **textures;

void loadTextures(void);
void destroyTextures(void);

#endif

