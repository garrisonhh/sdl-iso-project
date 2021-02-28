#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "render.h"
#include "vector.h"
#include "textures.h"
#include "sprites.h"

SDL_Renderer *renderer = NULL;
v2i camera = {0, 0};

const v2i SCREEN_CENTER = {SCREEN_WIDTH >> 2, SCREEN_HEIGHT >> 2};

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

void update_camera(world_t *world) {
	camera = v2i_sub(SCREEN_CENTER, v3d_to_isometric(world->player->ray.pos, false));
}

void render_entity(entity_t *entity) {
	v2i screen_pos = v3d_to_isometric(v3d_add(entity->ray.pos, v3d_scale(entity->size, -.5)), true);
	sprite_t *sprite = sprites[entity->sprite];
	render_sprite(sprite, screen_pos);
}

void render_chunk(chunk_t *chunk) {
	int x, y, z, index = 0, i;
	block_t *block;
	entity_bucket *bucket;
	v2i screen_pos;
	v3i block_loc;

	for (z = 0; z < SIZE; z++) {
		for (y = 0; y < SIZE; y++) {
			for (x = 0; x < SIZE; x++) {
				// entities
				bucket = chunk->buckets[index];
				if (bucket != NULL) {
					// TODO sort or presort multiple entities in a bucket
					for (i = 0; i < bucket->size; i++)
						render_entity(bucket->arr[i]);
				}

				// block
				block = chunk->blocks[index];
				if (block != NULL && block->expose_mask > 0) {
					block_loc = (v3i){x, y, z};
					block_loc = v3i_add(block_loc, v3i_scale(chunk->loc, SIZE));
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

				index++;
			}
		}
	}
}

void render_world(world_t *world) {
	for (int i = 0; i < world->num_chunks; i++) {
		render_chunk(world->chunks[i]);
	}
}

