#ifndef TEXTURES_H
#define TEXTURES_H

#include <SDL2/SDL.h>
#include <json-c/json.h>
#include "vector.h"
#include "data_structures/array.h"

// also modify textures_load when modifying this enum
enum texture_type_e {
	TEX_TEXTURE = 0,
	TEX_SPRITE = 1,
	TEX_VOXEL = 2,
	TEX_CONNECTED = 3,
	TEX_SHEET = 4,
};
typedef enum texture_type_e texture_type_e;   

// also modify textures_load when modifying this enum
// sprite types define the mapping of animation sheets; how they are ticked
enum sprite_type_e {
	SPRITE_STATIC = 0,
	SPRITE_HUMAN_BODY = 1,
	SPRITE_HUMAN_HANDS = 2,
	SPRITE_HUMAN_TOOL = 3,
};
typedef enum sprite_type_e sprite_type_e;

// texture types
// sprites require an animation_t for state as well, see animation.*
struct sprite_t {
	sprite_type_e type;

	// each row is an animation
	SDL_Texture *sheet;
	v2i pos, size;

	// array of animation lengths (corresponding to row lengths on sheet)
	int *anim_lengths;
	int num_anims;
};
typedef struct sprite_t sprite_t;

struct voxel_tex_t {
	// 7 textures drawn onto a single map, indexed by (block->expose_mask - 1)
	SDL_Texture *texture;
};
typedef struct voxel_tex_t voxel_tex_t;

struct connected_tex_t {
	SDL_Texture *directions[6];
	SDL_Texture *center;
};
typedef struct connected_tex_t connected_tex_t;

struct sheet_tex_t {
	SDL_Texture *sheet;
	v2i sheet_size;
};
typedef struct sheet_tex_t sheet_tex_t;

struct texture_t {
	texture_type_e type;
	bool transparent;
	size_t *tags;
	size_t num_tags;

	union texture_ptr {
		SDL_Texture *texture;

		sprite_t *sprite;

		voxel_tex_t *voxel;
		connected_tex_t *connected;
		sheet_tex_t *sheet;
	} tex;
};
typedef struct texture_t texture_t;

// texture state
union texture_state_t {
	// order is -X +X -Y +Y for top/bottom, corners are (-X, -Y) (+X -Y)...
	// bits 0-3 are top; 4-7 are bottom; 8-11 are corners
	unsigned outline_mask: 12;
	unsigned connected_mask: 6;
	v2i cell;
};
typedef union texture_state_t texture_state_t;

extern voxel_tex_t *DARK_VOXEL_TEXTURE;

void textures_load(void);
void textures_destroy(void);

SDL_Texture *load_sdl_texture(const char *path);

texture_t *texture_from_key(const char *);
sprite_t *sprite_from_key(const char *);
texture_state_t texture_state_from_type(texture_type_e tex_type);

#endif

