#ifndef TEXTURES_H
#define TEXTURES_H

#include <SDL2/SDL.h>
#include <json-c/json.h>
#include <stdint.h>
#include "vector.h"
#include "hash.h"

enum block_tex_type {
	TEX_TEXTURE = 0,
	TEX_VOXELTEXTURE = 1
};
typedef enum block_tex_type block_tex_type;   

struct vox_tex {
	SDL_Texture* top;
	SDL_Texture* side; // use SDL_RenderCopyEx to render flipped
};
typedef struct vox_tex vox_tex;

struct texture_t {
	block_tex_type type;
	SDL_Texture *texture;
	vox_tex *voxel_texture;
	bool transparent;
};
typedef struct texture_t texture_t;

extern texture_t **textures;

void textures_init(void);
void textures_load(json_object *);
void textures_destroy(void);
size_t texture_index(char *);
void render_tex_texture(SDL_Texture *, v2i);
vox_tex *load_voxel_texture(char *);
void render_voxel_texture(vox_tex *, v2i, uint8_t expose_mask);

#endif

