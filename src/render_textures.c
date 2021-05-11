#include <SDL2/SDL.h>
#include "textures.h"
#include "render.h"
#include "vector.h"

SDL_Rect SDL_TEX_RECT;
SDL_Rect VOXEL_TEX_RECTS[3];

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

// masks are 3-bit here
void render_voxel_texture(voxel_tex_t *voxel_texture, v2i pos, unsigned expose_mask, unsigned void_mask) {
	expose_mask = (expose_mask & ~void_mask) & 0x7;

	pos.x += VOXEL_TEX_RECTS[2].x;
	pos.y += VOXEL_TEX_RECTS[2].y;

	if (expose_mask)
		render_sdl_texture(voxel_texture->textures[expose_mask - 1], pos);

	if (void_mask)
		render_sdl_texture(VOID_VOXEL_TEXTURE->textures[void_mask - 1], pos);
}

void render_connected_texture(connected_tex_t *connected_tex, v2i pos, unsigned connected_mask) {
	SDL_Rect draw_rect = SDL_TEX_RECT;
	SDL_Texture *textures[6] = {
		connected_tex->bottom, connected_tex->top,
		connected_tex->back, connected_tex->front,
		connected_tex->back, connected_tex->front,
	};
	int i, j;

	draw_rect.x += pos.x;
	draw_rect.y += pos.y;

	SDL_RenderCopy(renderer, connected_tex->base, NULL, &draw_rect);

	for (i = 0; i <= 1; i++) {
		for (j = 0; j < 6; j += 2) {
			if ((connected_mask >> (i + j)) & 1) {
				if ((i + j) == 2 || (i + j) == 5)
					SDL_RenderCopyEx(renderer, textures[i + j], NULL, &draw_rect,
									 0, NULL, SDL_FLIP_HORIZONTAL);
				else
					SDL_RenderCopy(renderer, textures[i + j], NULL, &draw_rect);
			}
		}
	}
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
