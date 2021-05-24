#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "textures.h"
#include "world_masks.h"
#include "animation.h"

#define RENDER_FORMAT SDL_PIXELFORMAT_RGBA8888

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 800

#define VOXEL_WIDTH 32
#define VOXEL_HEIGHT 34
extern const int VOXEL_Z_HEIGHT;

extern SDL_Renderer *renderer;

typedef struct world_t world_t;

struct render_packet_t {
	v2i pos;
	texture_t *texture;

	union render_packet_state {
		texture_state_t tex;
		animation_t anim;
		voxel_masks_t voxel_masks;
		unsigned connected_mask: 6;
	} state;
};
typedef struct render_packet_t render_packet_t;

struct render_info_t {
	array_t **shadows;
	array_t **packets;
	size_t z_levels;
	int z_split;

	SDL_Rect cam_viewport;
};
typedef struct render_info_t render_info_t;

void render_init(SDL_Window *);
void render_quit(void);

render_info_t *render_gen_info(world_t *world);
void render_info_destroy(render_info_t *);

render_packet_t *render_packet_create(v2i pos, texture_t *texture);

void render_from_info(render_info_t *);

#endif
