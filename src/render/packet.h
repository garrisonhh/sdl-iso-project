#ifndef RENDER_PACKET_H
#define RENDER_PACKET_H

#include "primitives.h"
#include "../lib/vector.h"
#include "../textures.h"
#include "../world/masks.h"
#include "../sprites.h"
#include "../animation.h"

enum render_packet_e {
	RP_TEXTURE,
	RP_SPRITE,
	RP_SHADOW,
};
typedef enum render_packet_e render_packet_e;

typedef struct render_packet_data {
	render_packet_e type;
	v2i pos; // on screen
	v3i loc; // in world (for sorting)
} render_packet_data_t;

typedef struct texture_packet {
	render_packet_data_t _data;

	texture_t *texture;
	union {
		texture_state_t tex;
		voxel_masks_t voxel_masks;
		unsigned connected_mask: 6;
	} state;
} texture_packet_t;

typedef struct sprite_packet {
	render_packet_data_t _data;

	sprite_t *sprite;
	animation_t anim;
} sprite_packet_t;

typedef struct shadow_packet {
	render_packet_data_t _data;

	int radius;
} shadow_packet_t;

union render_packet_t {
	render_packet_data_t data;

	texture_packet_t texture;
	sprite_packet_t sprite;
	shadow_packet_t shadow;
};
typedef union render_packet_t render_packet_t;

render_packet_t *render_texture_packet_create(v3i loc, v2i pos, texture_t *);
render_packet_t *render_sprite_packet_create(v3i loc, v2i pos, sprite_t *);
render_packet_t *render_shadow_packet_create(v3i loc, v2i pos, int radius);

int render_packet_compare(const void *, const void *);
void render_from_packet(render_packet_t *);

#endif
