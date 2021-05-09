#ifndef TEXTURES_H
#define TEXTURES_H

#include <SDL2/SDL.h>
#include <json-c/json.h>
#include "vector.h"
#include "data_structures/array.h"

typedef struct entity_t entity_t;

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
	// each row is an animation
	SDL_Texture *sheet;
	v2i pos, size;

	// array of animation lengths (corresponding to row lengths on sheet)
	int *anim_lengths;
	int num_anims;
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

struct sheet_tex_t {
	SDL_Texture *sheet;
	v2i sheet_size;
};
typedef struct sheet_tex_t sheet_tex_t;

struct texture_t {
	texture_type_e type;
	bool transparent;

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
	unsigned outline_mask: 8;
	unsigned connected_mask: 6;
	v2i cell;
};
typedef union texture_state_t texture_state_t;

extern voxel_tex_t *VOID_VOXEL_TEXTURE;

void entity_anim_swap(entity_t *entity, int anim);
void entity_sprite_tick(entity_t *entity, double time);

void textures_load(void);
void textures_destroy(void);

texture_t *texture_from_key(char *);
texture_state_t texture_state_from_type(texture_type_e tex_type);

#endif

