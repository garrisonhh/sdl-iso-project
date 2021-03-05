#ifndef TEXTURES_H
#define TEXTURES_H

#include <SDL2/SDL.h>
#include <stdint.h>
#include "vector.h"

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
	vox_tex *voxel_texture;
	bool transparent;
} texture_t;

extern texture_t **textures;

void textures_init(void);
void textures_load(void);
void textures_destroy(void);
void render_tex_texture(SDL_Texture *, v2i);
vox_tex *load_voxel_texture(char *);
void render_voxel_texture(vox_tex *, v2i, Uint8 expose_mask);

#endif

