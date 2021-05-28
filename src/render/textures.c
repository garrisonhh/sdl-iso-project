#include <SDL2/SDL.h>
#include "textures.h"
#include "../render.h"
#include "../vector.h"
#include "../utils.h"
#include "../world_masks.h"
#include "../textures.h"
#include "../sprites.h"

int CONNECT_DRAW_ORDER[6] = {0, 2, 4, 1, 3, 5};

SDL_Rect SDL_TEX_RECT = {
	-(VOXEL_WIDTH >> 1),
	-VOXEL_Z_HEIGHT,
	VOXEL_WIDTH,
	VOXEL_HEIGHT
};
SDL_Rect VOXEL_TEX_RECTS[3] = {
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

const v2i OUTLINES[6][2] = {
	{ // top left
		(v2i){0, (VOXEL_WIDTH >> 2) - 2},
		(v2i){(VOXEL_WIDTH >> 1) - 1, -1}
	},
	{ // top right
		(v2i){VOXEL_WIDTH >> 1, -1},
		(v2i){VOXEL_WIDTH - 1, (VOXEL_WIDTH >> 2) - 2}
	},
	{ // bottom left
		(v2i){0, VOXEL_HEIGHT - (VOXEL_WIDTH >> 2) + 1},
		(v2i){(VOXEL_WIDTH >> 1) - 1, VOXEL_HEIGHT}
	},
	{ // bottom right
		(v2i){(VOXEL_WIDTH >> 1), VOXEL_HEIGHT},
		(v2i){VOXEL_WIDTH - 1, VOXEL_HEIGHT - (VOXEL_WIDTH >> 2) + 1}
	},
	{ // left corner
		(v2i){-1, (VOXEL_WIDTH >> 2) - 1},
		(v2i){-1, VOXEL_HEIGHT - (VOXEL_WIDTH >> 2) - 1}
	},
	{ // right corner
		(v2i){VOXEL_WIDTH, (VOXEL_WIDTH >> 2) - 1},
		(v2i){VOXEL_WIDTH, VOXEL_HEIGHT - (VOXEL_WIDTH >> 2) - 1}
	}
};

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
SDL_Texture *render_cached_voxel_texture(SDL_Surface *surfaces[3]) {
	int i;
	unsigned expose_mask;
	SDL_Surface *voxel_surface;
	SDL_Texture *texture;
	SDL_Rect dst_rect;

	voxel_surface = SDL_CreateRGBSurfaceWithFormat(0, VOXEL_WIDTH * 7, VOXEL_HEIGHT, 32, RENDER_FORMAT);

	// voxel shading, equivalent to applying (255, 255, 127) with alphas 31 and 63
	// this should correspond to shading in flat artwork
	SDL_SetSurfaceColorMod(surfaces[1], 0xDF, 0xDF, 0xEF);
	SDL_SetSurfaceColorMod(surfaces[0], 0xBF, 0xBF, 0xDF);

	for (expose_mask = 1; expose_mask < 8; ++expose_mask) {
		for (i = 0; i < 3; ++i) {
			if ((expose_mask >> i) & 1) {
				dst_rect = VOXEL_TEX_RECTS[i];
				dst_rect.x += VOXEL_WIDTH * (expose_mask - 1);

				SDL_BlitSurface(surfaces[i], NULL, voxel_surface, &dst_rect);
			}
		}
	}

	texture = SDL_CreateTextureFromSurface(renderer, voxel_surface);
	SDL_FreeSurface(voxel_surface);

	return texture;
}

// expose and void mask are processed to 3 bit: R-L-T
// outline mask is processed to 6 bits corresponding to OUTLINES array; see above
void render_voxel_texture(texture_t *texture, v2i pos, voxel_masks_t masks) {
	SDL_Rect dst_rect = SDL_TEX_RECT;
	SDL_Rect src_rect = {0, 0, VOXEL_WIDTH, VOXEL_HEIGHT};

	unsigned exposed = masks.expose & ((~masks.dark) & 0x7);

	dst_rect.x += pos.x;
	dst_rect.y += pos.y;
	src_rect.y = 0;

	if (exposed) {
		src_rect.x = VOXEL_WIDTH * (exposed - 1);
		SDL_RenderCopy(renderer, texture->texture, &src_rect, &dst_rect);
	}

	if (masks.dark) {
		src_rect.x = VOXEL_WIDTH * (masks.dark - 1);
		SDL_RenderCopy(renderer, DARK_VOXEL_TEXTURE->texture, &src_rect, &dst_rect);
	}

	if (masks.outline) {
		int i;
		v2i offset = {
			pos.x + SDL_TEX_RECT.x,
			pos.y + SDL_TEX_RECT.y
		};

		if (BIT_GET(masks.expose, 2)) {
			for (i = 0; i < 2; ++i) {
				if (BIT_GET(masks.outline, i)) {
					SDL_RenderDrawLine(renderer, offset.x + OUTLINES[i][0].x, offset.y + OUTLINES[i][0].y,
												 offset.x + OUTLINES[i][1].x, offset.y + OUTLINES[i][1].y);
				}
			}
		}

		for (i = 2; i < 4; ++i) {
			if (BIT_GET(masks.expose, 1 ^ (i & 1)) & BIT_GET(masks.outline, i)) {
				SDL_RenderDrawLine(renderer, offset.x + OUTLINES[i][0].x, offset.y + OUTLINES[i][0].y,
											 offset.x + OUTLINES[i][1].x, offset.y + OUTLINES[i][1].y);
			}
		}

		for (i = 4; i < 6; ++i) {
			if (BIT_GET(masks.expose, 1 ^ (i & 1)) & BIT_GET(masks.outline, i)) {
				SDL_RenderDrawLine(renderer,
								   offset.x + OUTLINES[i][0].x,
								   offset.y + OUTLINES[i][0].y,
								   offset.x + OUTLINES[i][1].x,
								   offset.y + OUTLINES[i][1].y + BIT_GET(masks.outline, i - 2));
			}
		}
	}
}

void render_connected_texture(texture_t *texture, v2i pos, unsigned connected_mask) {
	SDL_Rect dst_rect = SDL_TEX_RECT;
	SDL_Rect src_rect = dst_rect;

	dst_rect.x += pos.x;
	dst_rect.y += pos.y;

	src_rect.x = 6 * VOXEL_WIDTH;
	SDL_RenderCopy(renderer, texture->texture, &src_rect, &dst_rect);

	for (int i = 0; i < 6; i += 2) {
		if (BIT_GET(connected_mask, CONNECT_DRAW_ORDER[i])) {
			src_rect.x = VOXEL_WIDTH * CONNECT_DRAW_ORDER[i];
			SDL_RenderCopy(renderer, texture->texture, &src_rect, &dst_rect);
		}
	}
}

void render_sheet_texture(texture_t *texture, v2i pos, v2i cell) {
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

	SDL_RenderCopy(renderer, texture->texture, &cell_rect, &draw_rect);
}
