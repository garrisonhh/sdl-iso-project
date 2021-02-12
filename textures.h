#include <SDL2/SDL.h>

#ifndef TEXTURES_H
#define TEXTURES_H

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

#define NUM_TEXTURES 2
extern texture_t **textures;

void loadMedia(void);
void destroyMedia(void);

#endif

