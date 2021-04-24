#ifndef TEXTURES_H
#define TEXTURES_H

#include <SDL2/SDL.h>
#include <json-c/json.h>
#include "vector.h"

// numbers are used for json loading, if you change them make sure to
// check the textures_load function
enum texture_type_e {
	TEX_TEXTURE = 0,
	TEX_SPRITE = 1,
	TEX_VOXEL = 2,
	TEX_CONNECTED = 3,
	TEX_SHEET = 4,
};
typedef enum texture_type_e texture_type_e;   

// texture types
struct sprite_t {
	SDL_Texture *texture;
	v2i pos, size;
};
typedef struct sprite_t sprite_t;

struct voxel_tex_t {
	SDL_Texture *top, *side;
};
typedef struct voxel_tex_t voxel_tex_t;

struct connected_tex_t {
	SDL_Texture *base, *top, *bottom, *front, *back;
};
typedef struct connected_tex_t connected_tex_t;

// TODO separate sheet rows can have different lengths?
struct sheet_tex_t {
	SDL_Texture *texture;
	v2i size;
};
typedef struct sheet_tex_t sheet_tex_t;

struct texture_t {
	texture_type_e type;
	union texture_pointers {
		SDL_Texture *texture;
		sprite_t *sprite;
		voxel_tex_t *voxel;
		connected_tex_t *connected;
		sheet_tex_t *sheet;
	} tex;
	bool transparent;
};
typedef struct texture_t texture_t;

// texture state
struct block_tex_state_t {
	unsigned expose_mask: 3;

	union {
		unsigned outline_mask: 8;
		unsigned connected_mask: 6;
		v2i sheet_cell;
	} state;
};
typedef struct block_tex_state_t block_tex_state_t;

void textures_init(void);
void textures_load(json_object *);
void textures_destroy(void);
texture_t *texture_ptr_from_key(char *);
block_tex_state_t block_tex_state_from(texture_type_e tex_type);

void render_sdl_texture(SDL_Texture *, v2i);
void render_sprite(sprite_t *sprite, v2i);
void render_voxel_texture(voxel_tex_t *, v2i, unsigned expose_mask, unsigned void_mask);
void render_connected_texture(connected_tex_t *, v2i, unsigned connected_mask);
void render_sheet_texture(sheet_tex_t *, v2i, v2i cell);

#endif

