#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "world.h"
#include "render.h"
#include "textures.h"
#include "sprites.h"
#include "entity.h"

// TODO think about ways to refactor this file, has become more just "random
// SDL stuff" rather than "renderer stuff"

SDL_Renderer *renderer = NULL;
v2i camera = {0, 0};

const v2i SCREEN_CENTER = {SCREEN_WIDTH >> 2, SCREEN_HEIGHT >> 2};
const int VOXEL_Z_HEIGHT = VOXEL_HEIGHT - (VOXEL_WIDTH >> 1);
v2i tex_tex_offset = {
	-(VOXEL_WIDTH >> 1),
	-VOXEL_Z_HEIGHT
};
SDL_Rect vox_tex_rects[3] = { // used to render sides of vox_tex; t-l-r to correspond with bitmask rshifts
	{
		-(VOXEL_WIDTH >> 1),
		-VOXEL_Z_HEIGHT,
		VOXEL_WIDTH,
		VOXEL_WIDTH >> 1
	},
	{
		-(VOXEL_WIDTH >> 1),
		-VOXEL_Z_HEIGHT + (VOXEL_WIDTH >> 2),
		VOXEL_WIDTH >> 1,
		VOXEL_HEIGHT - (VOXEL_WIDTH >> 2)
	},
	{
		0,
		-VOXEL_Z_HEIGHT + (VOXEL_WIDTH >> 2),
		VOXEL_WIDTH >> 1,
		VOXEL_HEIGHT - (VOXEL_WIDTH >> 2)
	},
};
Uint8 vox_tex_shades[] = { // flat shading values (out of 255) for each side t-l-r
	255,
	223,
	191
};

void render_init(SDL_Window *window) {
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) {
		printf("unable to create renderer:\n%s\n", SDL_GetError());
		exit(1);
	}

	SDL_SetRenderDrawColor(renderer, 0x20, 0x20, 0x20, 0xFF);
	SDL_RenderSetScale(renderer, 2, 2); // TODO un-hardcode scaling
}

void render_destroy() {
	SDL_DestroyRenderer(renderer);
	renderer = NULL;
}

SDL_Texture *load_sdl_texture(char *path) {
	SDL_Texture *new_texture = NULL;
	SDL_Surface *loaded_surface = IMG_Load(path);
	if (loaded_surface == NULL) {
		printf("unable to load image %s:\n%s\n", path, IMG_GetError());
		exit(1);
	}

	new_texture = SDL_CreateTextureFromSurface(renderer, loaded_surface);
	if (new_texture == NULL) {
		printf("unable to create texture from %s:\n%s\n", path, SDL_GetError());
		exit(1);
	}

	SDL_FreeSurface(loaded_surface);
	return new_texture;
}

void media_load() {
	textures_load();
	sprites_load();
}

void media_destroy() {
	textures_destroy();
	sprites_destroy();
}

// TODO separate this vvv from this ^^^
void update_camera(world_t *world) {
	camera = v2i_sub(SCREEN_CENTER, v3d_to_isometric(world->player->pos, false));
}

// technically nothing to do with rendering, maybe move somewhere else?
SDL_Rect offset_rect(SDL_Rect *rect, v2i *offset) {
	SDL_Rect newRect = {
		offset->x + rect->x,
		offset->y + rect->y,
		rect->w,
		rect->h
	};
	return newRect;
}

// expose_mask uses only the last 3 bits; right-left-top order (corresponding to XYZ)
void render_voxel_texture(vox_tex *voxel_texture, v2i *pos, Uint8 expose_mask) {
	for (int i = 2; i >= 0; i--) {
		if ((expose_mask >> i) & 1) {
			SDL_Rect draw_rect = offset_rect(&vox_tex_rects[i], pos);
			Uint8 shade = vox_tex_shades[i];
			if (i == 0) {
				SDL_SetTextureColorMod(voxel_texture->top, shade, shade, shade);
			} else {
				SDL_SetTextureColorMod(voxel_texture->side, shade, shade, shade);
			}
			switch (i) {
			case 0:
				SDL_RenderCopy(renderer, voxel_texture->top, NULL, &draw_rect);
				break;
			case 1:
				SDL_RenderCopy(renderer, voxel_texture->side, NULL, &draw_rect);
				break;
			case 2:
				SDL_RenderCopyEx(renderer, voxel_texture->side, NULL, &draw_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
				break;
			}
		}
	}
}

// TODO entity sprite sorting into world
// TODO make entity Z level visually apparent
void render_entity(entity_t *entity) {
	v2i screen_pos = v3d_to_isometric(entity->pos, true);
	sprite_t *sprite = sprites[entity->sprite];
	SDL_Rect draw_rect = {
		screen_pos.x + sprite->x,
		screen_pos.y + sprite->y,
		sprite->w,
		sprite->h
	};
	SDL_RenderCopy(renderer, sprite->texture, NULL, &draw_rect);
}

void render_chunk(chunk_t *chunk) {
	int x, y, z, index = 0;
	block_t *block;
	v2i screen_pos;
	v3i block_loc;
	for (z = 0; z < SIZE; z++) {
		for (y = 0; y < SIZE; y++) {
			for (x = 0; x < SIZE; x++) {
				block = chunk->blocks[index++];
				if (block != NULL && block->expose_mask > 0) {
					block_loc = (v3i){
						x + chunk->loc.x * SIZE,
						y + chunk->loc.y * SIZE,
						z + chunk->loc.z * SIZE
					};
					screen_pos = v3i_to_isometric(block_loc, true);
					switch (textures[block->texture]->type) {
						case TEX_TEXTURE:
							; // yes, this semicolon is necessary to compile without errors. I am not joking.
							SDL_Rect draw_rect = {
								screen_pos.x + tex_tex_offset.x,
								screen_pos.y + tex_tex_offset.y,
								VOXEL_WIDTH,
								VOXEL_HEIGHT
							};
							SDL_RenderCopy(renderer, textures[block->texture]->texture, NULL, &draw_rect);
							break;
						case TEX_VOXELTEXTURE:
								render_voxel_texture(textures[block->texture]->voxel_texture, &screen_pos, block->expose_mask);
								break;
					}
				}
			}
		}
	}
}

void render_world(world_t *world) {
	for (int i = 0; i < world->num_chunks; i++) {
		render_chunk(world->chunks[i]);
	}

	render_entity(world->player); // TODO entity_t sorting into chunk rendering
}

