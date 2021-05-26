#ifndef RENDER_PACKET_H
#define RENDER_PACKET_H

#include "../vector.h"
#include "../textures.h"
#include "../world_masks.h"
#include "../sprites.h"
#include "../animation.h"

enum render_packet_e {
	RP_TEXTURE,
	RP_SPRITE,
	RP_SHADOW,
};
typedef enum render_packet_e render_packet_e;

struct render_packet_data_t {
	render_packet_e type;
	v2i pos;
};
typedef struct render_packet_data_t render_packet_data_t;

struct texture_packet_t {
	render_packet_data_t _data;

	texture_t *texture;
	union {
		texture_state_t tex;
		voxel_masks_t voxel_masks;
		unsigned connected_mask: 6;
	} state;
};
typedef struct texture_packet_t texture_packet_t;

struct sprite_packet_t {
	render_packet_data_t _data;

	sprite_t *sprite;
	animation_t anim;
};
typedef struct sprite_packet_t sprite_packet_t;

struct shadow_packet_t {
	render_packet_data_t _data;

	// TODO
};
typedef struct shadow_packet_t shadow_packet_t;

union render_packet_t {
	render_packet_data_t data;

	texture_packet_t texture;
	sprite_packet_t sprite;
	shadow_packet_t packet;
};
typedef union render_packet_t render_packet_t;

render_packet_t *render_texture_packet_create(v2i pos, texture_t *);
render_packet_t *render_sprite_packet_create(v2i pos, sprite_t *);

void render_from_packet(render_packet_t *);

#endif
