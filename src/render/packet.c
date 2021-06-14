#include <SDL2/SDL.h>
#include "packet.h"
#include "../render/textures.h"
#include "../render.h"
#include "../camera.h"
#include "../block/block.h"
#include "../entity/entity.h"

#define SHADOW_ALPHA (0x3F)

render_packet_t *render_texture_packet_create(v3i loc, v2i pos, texture_t *texture) {
	render_packet_t *packet = malloc(sizeof(render_packet_t));

	packet->data.type = RP_TEXTURE;
	packet->data.pos = pos;
	packet->data.loc = loc;

	packet->texture.texture = texture;

	return packet;
}

render_packet_t *render_sprite_packet_create(v3i loc, v2i pos, sprite_t *sprite) {
	render_packet_t *packet = malloc(sizeof(render_packet_t));

	packet->data.type = RP_SPRITE;
	packet->data.pos = pos;
	packet->data.loc = loc;

	packet->sprite.sprite = sprite;

	return packet;
}

render_packet_t *render_shadow_packet_create(v3i loc, v2i pos, int radius) {
	render_packet_t *packet = malloc(sizeof(render_packet_t));

	packet->data.type = RP_SHADOW;
	packet->data.pos = pos;
	packet->data.loc = loc;

	packet->shadow.radius = radius;

	return packet;
}

// for qsort
int render_packet_compare(const void *a, const void *b) {
	v3i loca, locb;

	loca = (*(render_packet_data_t **)a)->loc;
	locb = (*(render_packet_data_t **)b)->loc;

	loca = camera_rotated_v3i(loca);
	locb = camera_rotated_v3i(locb);

	return v3i_compare(loca, locb);
}

void render_from_packet(render_packet_t *packet) {
	switch (packet->data.type) {
	case RP_TEXTURE:
		switch (packet->texture.texture->type) {
		case TEX_TEXTURE:
			render_tex_texture(packet->texture.texture,
							   packet->data.pos);
			break;
		case TEX_VOXEL:
			render_voxel_texture(packet->texture.texture,
								 packet->data.pos,
								 packet->texture.state.voxel_masks);
			break;
		case TEX_CONNECTED:
			render_connected_texture(packet->texture.texture,
									 packet->data.pos,
									 packet->texture.state.connected_mask);
			break;
		case TEX_SHEET:
			render_sheet_texture(packet->texture.texture,
								 packet->data.pos,
								 packet->texture.state.tex.cell);
			break;
		default:
			break;
		}
		break;
	case RP_SPRITE:
		render_sprite(packet->sprite.sprite,
					  packet->data.pos,
					  packet->sprite.anim.cell);
		break;
	case RP_SHADOW:
		SDL_SetRenderDrawColor(RENDERER, 0, 0, 0, SHADOW_ALPHA);
		circle_t circle = {packet->data.pos, packet->shadow.radius};
		render_iso_circle(circle);
		break;
	}
}
