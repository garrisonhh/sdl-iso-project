#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "render.h"
#include "textures.h"
#include "sprites.h"
#include "vector.h"
#include "list.h"
#include "utils.h"

#define BG_GRAY 31
#define SHADOW_ALPHA 63

const int VOXEL_Z_HEIGHT = VOXEL_HEIGHT - (VOXEL_WIDTH >> 1);

SDL_Renderer *renderer = NULL;

v2i camera = {0, 0};
int camera_scale;
v2i screen_center;

void render_init(SDL_Window *window) {
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) {
		printf("unable to create renderer:\n%s\n", SDL_GetError());
		exit(1);
	}

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, BG_GRAY, BG_GRAY, BG_GRAY, 0xFF);
	SDL_RenderSetIntegerScale(renderer, true);

	camera_set_scale(2);
}

void render_destroy() {
	SDL_DestroyRenderer(renderer);
	renderer = NULL;
}

void render_clear_screen() {
	SDL_SetRenderDrawColor(renderer, BG_GRAY, BG_GRAY, BG_GRAY, 0xFF);
	SDL_RenderClear(renderer);
}

v2i v3i_to_isometric(v3i v, bool at_camera) {
	v2i iso = {
		((v.x - v.y) * VOXEL_WIDTH) >> 1,
		(((v.x + v.y) * VOXEL_WIDTH) >> 2) - (v.z * VOXEL_Z_HEIGHT)
	};

	if (at_camera)
		return v2i_add(iso, camera);
	return iso;
}

v2i v3d_to_isometric(v3d v, bool at_camera) {
	v2i iso = {
		((v.x - v.y) * VOXEL_WIDTH) / 2,
		(((v.x + v.y) * VOXEL_WIDTH) / 4) - (v.z * VOXEL_Z_HEIGHT)
	};

	if (at_camera)
		return v2i_add(iso, camera);
	return iso;
}

void camera_update(world_t *world) {
	camera = v2i_sub(screen_center, v3d_to_isometric(world->player->ray.pos, false));
}

void camera_set_scale(int scale) {
	camera_scale = scale;
	screen_center = (v2i){(SCREEN_WIDTH / camera_scale) >> 1, (SCREEN_HEIGHT / camera_scale) >> 1};
	SDL_RenderSetScale(renderer, camera_scale, camera_scale);
}

void render_entity(entity_t *entity) {
	v2i screen_pos = v3d_to_isometric(v3d_add(entity->ray.pos, v3d_scale(entity->size, -.5)), true);
	sprite_t *sprite = sprites[entity->sprite];
	render_sprite(sprite, screen_pos);
}

// renders 2-1 ellipse at center
void render_shadow(v2i center, int r) {
	int i, rx = r, ry = r >> 1, ix = r;
	float y = 0.0, y_step = 1 / (float)ry;

	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, SHADOW_ALPHA);
	SDL_RenderDrawLine(renderer, center.x + r, center.y,
								 center.x - r, center.y);

	for (i = 1; i <= ry; i++) {
		ix = (int)(sqrt(1 - (y * y)) * rx);
		y += y_step;

		SDL_RenderDrawLine(renderer, center.x + ix, center.y + i,
									 center.x - ix, center.y + i);
		SDL_RenderDrawLine(renderer, center.x + ix, center.y - i,
									 center.x - ix, center.y - i);
	}
}

void render_world(world_t *world) {
	int cx, cy, cz, x, y, z, i;
	int chunk_origin = 0, block_origin = 0;
	int chunk_index = 0, block_index = 0;
	int chunk_z_step = world->dims.y * world->dims.x, block_z_step = SIZE * SIZE;
	chunk_t *chunk;
	block_t *block;
	list_t *bucket;
	v2i screen_pos;
	v3i block_loc;

	for (cz = 0; cz < world->dims.z; cz++) {
		block_origin = 0;

		for (z = 0; z < SIZE; z++) {
			chunk_index = chunk_origin;

			FOR_XY (cx, cy, world->dims.x, world->dims.y) {
				chunk = world->chunks[chunk_index++];
				block_index = block_origin;

				FOR_XY (x, y, SIZE, SIZE) {
					bucket = chunk->buckets[block_index];
					block = chunk->blocks[block_index];
					block_index++;

					if (bucket != NULL) {
						for (i = 0; i < bucket->size; i++)
							render_entity(bucket->items[i]);
					}
					
					if (block != NULL && block->expose_mask > 0) {
						block_loc = (v3i){cx * SIZE + x, cy * SIZE + y, cz * SIZE + z};
						screen_pos = v3i_to_isometric(block_loc, true);

						switch (textures[block->texture]->type) {
							case TEX_TEXTURE:
								render_tex_texture(textures[block->texture]->texture, screen_pos);
								break;
							case TEX_VOXELTEXTURE:
								render_voxel_texture(textures[block->texture]->voxel_texture,
													 screen_pos,
													 block->expose_mask);
								break;
						}
					}
				}
			}

			block_origin += block_z_step;
		}

		chunk_origin += chunk_z_step;
	}
}
