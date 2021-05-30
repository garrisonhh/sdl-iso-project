#ifndef TEXTURES_H
#define TEXTURES_H

#include <SDL2/SDL.h>
#include <json-c/json.h>
#include "lib/vector.h"
#include "lib/array.h"

// also modify textures_load when modifying this enum
enum texture_type_e {
	TEX_TEXTURE,
	TEX_VOXEL,
	TEX_CONNECTED,
	TEX_SHEET,
	
	NUM_TEXTURE_TYPES
};
typedef enum texture_type_e texture_type_e;   

struct texture_t {
	texture_type_e type;
	// voxel: 1x7; indexed by (block->expose_mask - 1)
	// connected: 1x7; 0-5 are directions, 6 is center
	SDL_Texture *texture;

	bool transparent;
	size_t *tags;
	size_t num_tags;
};
typedef struct texture_t texture_t;

union texture_state_t {
	// order is -X +X -Y +Y for top/bottom, corners are (-X, -Y) (+X -Y)...
	// bits 0-3 are top; 4-7 are bottom; 8-11 are corners
	unsigned outline_mask: 12;
	// -X +X -Y +Y -Z +Z
	unsigned connected_mask: 6;
	v2i cell;
};
typedef union texture_state_t texture_state_t;

extern texture_t *DARK_VOXEL_TEXTURE;

void textures_load(void);
void textures_destroy(void);

SDL_Texture *load_sdl_texture(const char *path);

texture_t *texture_from_key(const char *);
texture_state_t texture_state_from_type(texture_type_e tex_type);

#endif

