#ifndef TEXTURES_H
#define TEXTURES_H

#include <SDL2/SDL.h>
#include <json-c/json.h>
#include <stdint.h>
#include "vector.h"
#include "hash.h"

enum texture_type {
	TEX_TEXTURE = 0,
	TEX_VOXEL = 1,
	TEX_CONNECTED = 2,
};
typedef enum texture_type texture_type;   

struct voxel_tex_t {
	SDL_Texture *top, *side;
};
typedef struct voxel_tex_t voxel_tex_t;

struct connected_tex_t {
	SDL_Texture *base, *top, *bottom, *front, *back;
};
typedef struct connected_tex_t connected_tex_t;

struct texture_t {
	texture_type type;
	SDL_Texture *texture;
	voxel_tex_t *voxel_tex;
	connected_tex_t *connected_tex;
	bool transparent;
};
typedef struct texture_t texture_t;

extern texture_t **textures;

void textures_init(void);
void textures_load(json_object *);
void textures_destroy(void);
size_t texture_index(char *);
void render_sdl_texture(SDL_Texture *, v2i);
voxel_tex_t *load_voxel_texture(char *);
void render_voxel_texture(voxel_tex_t *, v2i, uint8_t, uint8_t);
connected_tex_t *load_connected_texture(char *);
void render_connected_texture(connected_tex_t *, v2i, uint8_t);

#endif

