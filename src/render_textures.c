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
			0,
			-VOXEL_Z_HEIGHT + (VOXEL_WIDTH >> 2),
			VOXEL_WIDTH >> 1,
			VOXEL_HEIGHT - (VOXEL_WIDTH >> 2)
		},
		{
			-(VOXEL_WIDTH >> 1),
			-VOXEL_Z_HEIGHT + (VOXEL_WIDTH >> 2),
			VOXEL_WIDTH >> 1,
			VOXEL_HEIGHT - (VOXEL_WIDTH >> 2)
		},
		{
			-(VOXEL_WIDTH >> 1),
			-VOXEL_Z_HEIGHT,
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

void render_sprite(sprite_t *sprite, v2i pos) {
	pos = v2i_add(pos, sprite->pos);
	SDL_Rect draw_rect = {
		pos.x, pos.y,
		sprite->size.x, sprite->size.y
	};

	SDL_RenderCopy(renderer, sprite->texture, NULL, &draw_rect);
}

// these masks use only the last 3 bits, ZYX (to match indexing (v3i){x, y, z})
// void_mask determines sides which will be displayed as void (fully black)
void render_voxel_texture(voxel_tex_t *voxel_texture, v2i pos, unsigned expose_mask, unsigned void_mask) {
	SDL_Rect draw_rect;
	voxel_tex_t *cur_texture;

	for (int i = 0; i < 3; ++i) {
		if ((expose_mask >> i) & 1 || (void_mask >> i) & 1) {//(expose_mask | void_mask >> i) & 1) {
			draw_rect = VOXEL_TEX_RECTS[i];
			draw_rect.x += pos.x;
			draw_rect.y += pos.y;

			cur_texture = ((void_mask >> i) & 1 ? VOID_VOXEL_TEXTURE : voxel_texture);

			switch (i) {
				case 0:
					SDL_RenderCopyEx(renderer, cur_texture->side, NULL, &draw_rect,
							         0, NULL, SDL_FLIP_HORIZONTAL);
					break;
				case 1:
					SDL_RenderCopy(renderer, cur_texture->side, NULL, &draw_rect);
					break;
				case 2:
					SDL_RenderCopy(renderer, cur_texture->top, NULL, &draw_rect);
					break;
			}
		}
	}
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

	SDL_RenderCopy(renderer, sheet_tex->texture, &cell_rect, &draw_rect);
}
