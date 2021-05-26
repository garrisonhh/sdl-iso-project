#include <SDL2/SDL.h>
#include "packet.h"
#include "textures.h"
#include "../render.h"
#include "../block.h"
#include "../entity.h"

#define SHADOW_ALPHA (0x3F)

render_packet_t *render_texture_packet_create(v2i pos, int z, texture_t *texture) {
	render_packet_t *packet = malloc(sizeof(render_packet_t));

	packet->data.type = RP_TEXTURE;
	packet->data.pos = pos;
	packet->data.z = z;

	packet->texture.texture = texture;

	return packet;
}

render_packet_t *render_sprite_packet_create(v2i pos, int z, sprite_t *sprite) {
	render_packet_t *packet = malloc(sizeof(render_packet_t));

	packet->data.type = RP_SPRITE;
	packet->data.pos = pos;
	packet->data.z = z;

	packet->sprite.sprite = sprite;

	return packet;
}

render_packet_t *render_shadow_packet_create(v2i pos, int z, int radius) {
	render_packet_t *packet = malloc(sizeof(render_packet_t));

	packet->data.type = RP_SHADOW;
	packet->data.pos = pos;
	packet->data.z = z;

	packet->shadow.radius = radius;

	return packet;
}

void render_from_packet(render_packet_t *packet) {
	switch (packet->data.type) {
		case RP_TEXTURE:
			switch (packet->texture.texture->type) {
				case TEX_TEXTURE:
					render_sdl_texture(packet->texture.texture->texture,
									   packet->data.pos);
					break;
				case TEX_VOXEL:
					// TODO voxel outline colors?
					SDL_SetRenderDrawColor(renderer, BG_GRAY, BG_GRAY, BG_GRAY, 0xFF);
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
			}
			break;
		case RP_SPRITE:
			render_sprite(packet->sprite.sprite,
						  packet->data.pos,
						  packet->sprite.anim.cell);
			break;
		case RP_SHADOW:
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, SHADOW_ALPHA);

			circle_t circle = {packet->data.pos, packet->shadow.radius};
			render_iso_circle(circle);
			break;
	}
}

