#include <SDL2/SDL.h>
#include "textures.h"
#include "render.h"
#include "vector.h"
#include "utils.h"
#include "world_masks.h"
#include "render_textures.h"

SDL_Rect SDL_TEX_RECT;
SDL_Rect VOXEL_TEX_RECTS[3];

const v2i OUTLINES[6][2] = {
	{ // top left
		(v2i){0, (VOXEL_WIDTH >> 2) - 1},
		(v2i){(VOXEL_WIDTH >> 1) - 1, 0}
	},
	{ // top right
		(v2i){(VOXEL_WIDTH >> 1), 0},
		(v2i){VOXEL_WIDTH - 1, (VOXEL_WIDTH >> 2) - 1}
	},
	{ // bottom left
		(v2i){0, VOXEL_HEIGHT - (VOXEL_WIDTH >> 2)},
		(v2i){(VOXEL_WIDTH >> 1) - 1, VOXEL_HEIGHT - 1}
	},
	{ // bottom right
		(v2i){(VOXEL_WIDTH >> 1), VOXEL_HEIGHT - 1},
		(v2i){VOXEL_WIDTH - 1, VOXEL_HEIGHT - (VOXEL_WIDTH >> 2)}
	},
	{ // left corner
		(v2i){0, VOXEL_WIDTH >> 2},
		(v2i){0, VOXEL_HEIGHT - (VOXEL_WIDTH >> 2) - 1}
	},
	{ // right corner
		(v2i){VOXEL_WIDTH - 1, VOXEL_WIDTH >> 2},
		(v2i){VOXEL_WIDTH - 1, VOXEL_HEIGHT - (VOXEL_WIDTH >> 2) - 1}
	}
};

// workaround for C's weird global constant rules
void render_textures_init() {
	SDL_TEX_RECT = (SDL_Rect){
		-(VOXEL_WIDTH >> 1),
		-VOXEL_Z_HEIGHT,
		VOXEL_WIDTH,
		VOXEL_HEIGHT
	};

	SDL_Rect voxel_tex_rects_tmp[3] = {
		{
			VOXEL_WIDTH >> 1,
			VOXEL_WIDTH >> 2,
			VOXEL_WIDTH >> 1,
			VOXEL_Z_HEIGHT
		},
		{
			0,
			VOXEL_WIDTH >> 2,
			VOXEL_WIDTH >> 1,
			VOXEL_Z_HEIGHT
		},
		{
			0,
			0,
			VOXEL_WIDTH,
			VOXEL_WIDTH >> 1
		},
	};

	memcpy(VOXEL_TEX_RECTS, voxel_tex_rects_tmp, sizeof voxel_tex_rects_tmp);
}

void render_render_packet(render_packet_t *packet) {
	switch (packet->texture->type) {
		case TEX_TEXTURE:
			render_sdl_texture(packet->texture->tex.texture, packet->pos);
			break;
		case TEX_SPRITE:
			render_sprite(packet->texture->tex.sprite, packet->pos, packet->state.anim.cell);
			break;
		case TEX_VOXEL:
			render_voxel_texture(packet->texture->tex.voxel, packet->pos, packet->state.voxel_masks);
			break;
		case TEX_CONNECTED:
			render_connected_texture(packet->texture->tex.connected, packet->pos,
									 packet->state.tex.connected_mask);
			break;
		case TEX_SHEET:
			render_sheet_texture(packet->texture->tex.sheet, packet->pos, packet->state.tex.cell);
			break;
	}
}

void render_sdl_texture(SDL_Texture *texture, v2i pos) {
	SDL_Rect draw_rect = SDL_TEX_RECT;
	draw_rect.x += pos.x;
	draw_rect.y += pos.y;

	SDL_RenderCopy(renderer, texture, NULL, &draw_rect);
}

void render_sprite(sprite_t *sprite, v2i pos, v2i cell) {
	SDL_Rect src_rect = {
		cell.x * sprite->size.x,
		cell.y * sprite->size.y,
		sprite->size.x,
		sprite->size.y
	};
	SDL_Rect dst_rect = {
		pos.x + sprite->pos.x,
		pos.y + sprite->pos.y,
		sprite->size.x,
		sprite->size.y
	};

	SDL_RenderCopy(renderer, sprite->sheet, &src_rect, &dst_rect);
}

void render_sprite_no_offset(sprite_t *sprite, v2i pos, v2i cell) {
	SDL_Rect src_rect = {
		cell.x * sprite->size.x,
		cell.y * sprite->size.y,
		sprite->size.x,
		sprite->size.y
	};
	SDL_Rect dst_rect = {
		pos.x,
		pos.y,
		sprite->size.x,
		sprite->size.y
	};

	SDL_RenderCopy(renderer, sprite->sheet, &src_rect, &dst_rect);
}

// surfaces are in right-left-top order
SDL_Texture *render_cached_voxel_texture(SDL_Surface *surfaces[3], unsigned expose_mask) {
	SDL_Surface *voxel_surface;
	SDL_Texture *texture;

	voxel_surface = SDL_CreateRGBSurfaceWithFormat(0, VOXEL_WIDTH, VOXEL_HEIGHT, 32, RENDER_FORMAT);

	// voxel shading, equivalent to applying (255, 255, 127) with alphas 31 and 63
	// this should correspond to shading in flat artwork
	SDL_SetSurfaceColorMod(surfaces[1], 0xDF, 0xDF, 0xEF);
	SDL_SetSurfaceColorMod(surfaces[0], 0xBF, 0xBF, 0xDF);

	for (int i = 0; i < 3; ++i)
		if ((expose_mask >> i) & 1)
			SDL_BlitSurface(surfaces[i], NULL, voxel_surface, &VOXEL_TEX_RECTS[i]);

	texture = SDL_CreateTextureFromSurface(renderer, voxel_surface);
	SDL_FreeSurface(voxel_surface);

	return texture;
}

// expose and void mask are processed to 3 bit: R-L-T
// outline mask is processed to 6 bits corresponding to OUTLINES array; see above
void render_voxel_texture(voxel_tex_t *voxel_texture, v2i pos, voxel_masks_t masks) {
	masks.expose = masks.expose & ((~masks.dark) & 0x7);

	if (masks.expose) {
		SDL_RenderDrawPoint(renderer, pos.x, pos.y);
		render_sdl_texture(voxel_texture->textures[masks.expose - 1], pos);
	}

	if (masks.dark)
		render_sdl_texture(DARK_VOXEL_TEXTURE->textures[masks.dark - 1], pos);

	if (masks.outline) {
		int i;
		v2i offset = {
			pos.x + SDL_TEX_RECT.x,
			pos.y + SDL_TEX_RECT.y
		};

		for (i = 0; i < 2; ++i) {
			if (BIT_GET(masks.outline, i)) {
				SDL_RenderDrawLine(renderer, offset.x + OUTLINES[i][0].x, offset.y + OUTLINES[i][0].y,
											 offset.x + OUTLINES[i][1].x, offset.y + OUTLINES[i][1].y);
			}
		}

		for (i = 2; i < 6; ++i) {
			// TODO check side face exposure
			if (BIT_GET(masks.outline, i)) {
				SDL_RenderDrawLine(renderer, offset.x + OUTLINES[i][0].x, offset.y + OUTLINES[i][0].y,
											 offset.x + OUTLINES[i][1].x, offset.y + OUTLINES[i][1].y);
			}
		}
	}
}

void render_connected_texture(connected_tex_t *connected_tex, v2i pos, unsigned connected_mask) {
	int i;
	SDL_Rect draw_rect = SDL_TEX_RECT;

	draw_rect.x += pos.x;
	draw_rect.y += pos.y;

	SDL_RenderCopy(renderer, connected_tex->center, NULL, &draw_rect);

	// negative dirs
	for (i = 0; i < 6; i += 2)
		if (BIT_GET(connected_mask, i))
			SDL_RenderCopy(renderer, connected_tex->directions[i], NULL, &draw_rect);

	// positive dirs
	for (i = 1; i < 6; i += 2)
		if (BIT_GET(connected_mask, i))
			SDL_RenderCopy(renderer, connected_tex->directions[i], NULL, &draw_rect);
}

void render_sheet_texture(sheet_tex_t *sheet_tex, v2i pos, v2i cell) {
	SDL_Rect draw_rect, cell_rect;

	draw_rect = SDL_TEX_RECT;
	draw_rect.x += pos.x;
	draw_rect.y += pos.y;

	cell_rect = (SDL_Rect){
		cell.x * VOXEL_WIDTH,
		cell.y * VOXEL_HEIGHT,
		VOXEL_WIDTH,
		VOXEL_HEIGHT
	};

	SDL_RenderCopy(renderer, sheet_tex->sheet, &cell_rect, &draw_rect);
}
