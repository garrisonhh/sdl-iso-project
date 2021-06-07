#include <SDL2/SDL.h>
#include "textures.h"
#include "../render.h"
#include "../lib/utils.h"

const int CONNECT_DRAW_ORDER[6] = {0, 2, 4, 1, 3, 5};

const SDL_Rect SDL_TEX_RECT = {
	-(VOXEL_WIDTH >> 1),
	-VOXEL_Z_HEIGHT,
	VOXEL_WIDTH,
	VOXEL_HEIGHT
};
const SDL_Rect VOXEL_TEX_RECTS[3] = {
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

void render_sdl_texture(SDL_Texture *texture, v2i pos) {
	SDL_Rect draw_rect = SDL_TEX_RECT;
	draw_rect.x += pos.x;
	draw_rect.y += pos.y;

	SDL_RenderCopy(RENDERER, texture, NULL, &draw_rect);
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

	SDL_RenderCopy(RENDERER, sprite->sheet, &src_rect, &dst_rect);
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

	SDL_RenderCopy(RENDERER, sprite->sheet, &src_rect, &dst_rect);
}

// surfaces are in right-left-top order
SDL_Surface *render_cached_voxel_surface(SDL_Surface *surfaces[3]) {
	int i;
	unsigned expose_mask;
	SDL_Surface *voxel_surface;
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

	return voxel_surface;
}

SDL_Surface *render_voxel_outline_surface(SDL_Surface *outlines) {
	int i;
	int variations = 1 << 6;
	SDL_Surface *voxel_surface;
	SDL_Rect src_rect, dst_rect;

	src_rect = (SDL_Rect){0, 0, VOXEL_WIDTH + 2, VOXEL_HEIGHT + 2};
	dst_rect = src_rect;

	voxel_surface = SDL_CreateRGBSurfaceWithFormat(0,
												   src_rect.w * variations,
												   src_rect.h,
												   32,
												   RENDER_FORMAT);

	for (unsigned mask = 1; mask < variations; ++mask) {
		src_rect.x = 0;

		for (i = 0; i < 6; ++i) {
			if (BIT_GET(mask, i))
				SDL_BlitSurface(outlines, &src_rect, voxel_surface, &dst_rect);

			src_rect.x += src_rect.w;
		}

		dst_rect.x += dst_rect.w;
	}

	return voxel_surface;
}

void render_tex_texture(texture_t *texture, v2i pos) {
	SDL_Rect draw_rect = SDL_TEX_RECT;
	draw_rect.x += pos.x;
	draw_rect.y += pos.y;

	SDL_RenderCopy(RENDERER, TEXTURE_ATLAS, &texture->atlas_rect, &draw_rect);
}

// expose and void mask are processed to 3 bit: R-L-T
// outline mask is processed to 6 bits corresponding to OUTLINES array; see above
void render_voxel_texture(texture_t *texture, v2i pos, voxel_masks_t masks) {
	SDL_Rect dst_rect = SDL_TEX_RECT;
	SDL_Rect src_rect = texture->atlas_rect;
	unsigned exposed = masks.expose & ((~masks.dark) & 0x7);

	dst_rect.x += pos.x;
	dst_rect.y += pos.y;

	if (exposed) {
		src_rect.x = VOXEL_WIDTH * (exposed - 1);
		SDL_RenderCopy(RENDERER, TEXTURE_ATLAS, &src_rect, &dst_rect);
	}

	if (masks.dark) {
		src_rect = DARK_VOXEL_TEXTURE->atlas_rect;
		src_rect.x = VOXEL_WIDTH * (masks.dark - 1);
		SDL_RenderCopy(RENDERER, TEXTURE_ATLAS, &src_rect, &dst_rect);
	}

	if (masks.outline) {
		dst_rect.x -= 1;
		dst_rect.y -= 1;
		dst_rect.w = TEXTURE_OUTLINES_RECT.w;
		dst_rect.h = TEXTURE_OUTLINES_RECT.h;

		src_rect = TEXTURE_OUTLINES_RECT;
		src_rect.x = (masks.outline - 1) * src_rect.w;

		SDL_RenderCopy(RENDERER, TEXTURE_ATLAS, &src_rect, &dst_rect);
	}
}

void render_connected_texture(texture_t *texture, v2i pos, unsigned connected_mask) {
	SDL_Rect dst_rect = SDL_TEX_RECT;
	SDL_Rect src_rect = texture->atlas_rect;

	dst_rect.x += pos.x;
	dst_rect.y += pos.y;

	src_rect.x += 6 * VOXEL_WIDTH;

	SDL_RenderCopy(RENDERER, TEXTURE_ATLAS, &src_rect, &dst_rect);

	for (int i = 0; i < 6; ++i) {
		if (BIT_GET(connected_mask, CONNECT_DRAW_ORDER[i])) {
			src_rect.x = texture->atlas_rect.x + VOXEL_WIDTH * CONNECT_DRAW_ORDER[i];

			SDL_RenderCopy(RENDERER, TEXTURE_ATLAS, &src_rect, &dst_rect);
		}
	}
}

void render_sheet_texture(texture_t *texture, v2i pos, v2i cell) {
	SDL_Rect dst_rect = SDL_TEX_RECT;
	SDL_Rect src_rect = texture->atlas_rect;

	dst_rect.x += pos.x;
	dst_rect.y += pos.y;

	src_rect.x += cell.x * VOXEL_WIDTH;
	src_rect.y += cell.y * VOXEL_HEIGHT;

	SDL_RenderCopy(RENDERER, TEXTURE_ATLAS, &src_rect, &dst_rect);
}
