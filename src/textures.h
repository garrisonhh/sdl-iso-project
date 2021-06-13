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

/*
 * voxel: 1x7 on atlas; x indexed by (block->expose_mask - 1)
 * connected: 1x7 on atlas; x = 0-5 are directions, x = 6 is center
 *
 * voxel, connected, and sheet textures can be indexed by adding rect x, y to
 * index x, y multiplied by rect w, h. (rect w, h indicate size of a cell, not
 * size of whole texture);
 */
struct texture_t {
	texture_type_e type;
	SDL_Rect atlas_rect;

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

extern const SDL_Rect TEXTURE_OUTLINES_RECT;
extern texture_t *DARK_VOXEL_TEXTURE;
extern SDL_Texture *TEXTURE_ATLAS;

void textures_load(void);
void textures_destroy(void);

SDL_Texture *load_sdl_texture(const char *path);

texture_t *texture_from_key(const char *);

#endif
