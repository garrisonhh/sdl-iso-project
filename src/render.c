#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "render.h"
#include "textures.h"
#include "vector.h"
#include "collision.h"
#include "raycast.h"
#include "list.h"
#include "camera.h"
#include "render_primitives.h"
#include "utils.h"
#include "world.h"

#define BG_GRAY 31
#define SHADOW_ALPHA 63

const int VOXEL_Z_HEIGHT = VOXEL_HEIGHT - (VOXEL_WIDTH >> 1);
const v3d PLAYER_VIEW_DIR = {VOXEL_HEIGHT, VOXEL_HEIGHT, VOXEL_WIDTH};

SDL_Renderer *renderer = NULL;
SDL_Texture *foreground = NULL, *background = NULL;

void render_init(SDL_Window *window) {
	renderer = SDL_CreateRenderer(window, -1,
								  SDL_RENDERER_ACCELERATED
								  | SDL_RENDERER_PRESENTVSYNC
								  | SDL_RENDERER_TARGETTEXTURE);
	if (renderer == NULL) {
		printf("unable to create renderer:\n%s\n", SDL_GetError());
		exit(1);
	}

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, BG_GRAY, BG_GRAY, BG_GRAY, 0xFF);
	SDL_RenderSetIntegerScale(renderer, true);

	foreground = SDL_CreateTexture(renderer,
								   SDL_PIXELFORMAT_RGBA8888,
								   SDL_TEXTUREACCESS_TARGET,
								   SCREEN_WIDTH,
								   SCREEN_HEIGHT);
	background = SDL_CreateTexture(renderer,
								   SDL_PIXELFORMAT_RGBA8888,
								   SDL_TEXTUREACCESS_TARGET,
								   SCREEN_WIDTH,
								   SCREEN_HEIGHT);
	SDL_SetTextureBlendMode(foreground, SDL_BLENDMODE_BLEND);
	SDL_SetTextureBlendMode(foreground, SDL_BLENDMODE_BLEND);

	camera_init();
}

void render_destroy() {
	SDL_DestroyRenderer(renderer);
	renderer = NULL;
	SDL_DestroyTexture(foreground);
	SDL_DestroyTexture(background);
	foreground = NULL;
	background = NULL;
}

void render_generate_shadows(world_t *world, list_t *(*shadows)[world->block_size]) {
	int z, i;
	entity_t *entity;
	block_t *block;
	circle_t *shadow;
	v3d shadow_pos;
	v3i shadow_loc;

	// generate *shadows and z sort
	for (z = 0; z < world->block_size; z++)
		(*shadows)[z] = NULL;

	for (i = 0; i < world->entities->size; i++) {
		entity = world->entities->items[i];
		shadow_pos = entity->ray.pos;
		shadow_pos.z -= entity->size.z / 2;
		shadow_loc = v3i_from_v3d(shadow_pos);

		while (shadow_loc.z >= 0) {
			if ((block = block_get(world, shadow_loc)) != NULL
			 && !block->texture->transparent) {
				break;
			}
			shadow_loc.z--;
		}

		shadow_loc.z++;
		shadow_pos.z = shadow_loc.z;

		if (shadow_loc.z >= 0 && shadow_loc.z < world->block_size) {
			shadow = (circle_t *)malloc(sizeof(circle_t));
			shadow->loc = project_v3d(shadow_pos, true);
			shadow->radius = (int)((entity->size.x * VOXEL_WIDTH) / 2.0);

			if ((*shadows)[shadow_loc.z] == NULL)
				(*shadows)[shadow_loc.z] = list_create();

			list_add((*shadows)[shadow_loc.z], shadow);
		}
	}
}

void render_entity(entity_t *entity) {
	v3d entity_pos;
	v2i screen_pos;

	entity_pos = entity->ray.pos;
	entity_pos.z -= entity->size.z / 2;
	screen_pos = project_v3d(entity_pos, true);

	render_sprite(entity->sprite->tex.sprite, screen_pos);
}

void render_block(world_t *world, block_t *block, v3i loc, uint8_t void_mask) {
	texture_type tex_type = block->texture->type;

	if (tex_type == TEX_VOXEL) {
		if (block->expose_mask || void_mask) {
			render_voxel_texture(block->texture->tex.voxel,
								 project_v3i(loc, true),
								 block->expose_mask, void_mask);
		}
	} else if (block->expose_mask) {
		// the amount of times I had to type "tex" or "texture" here is hilarious lol
		switch (tex_type) {
			case TEX_TEXTURE:
				render_sdl_texture(block->texture->tex.texture, project_v3i(loc, true));

				break;
			case TEX_CONNECTED:
				render_connected_texture(block->texture->tex.connected,
										 project_v3i(loc, true),
										 block->connect_mask);

				break;
			default:
				break;
		}
	}
}

void render_world(world_t *world) {
	int x, y, z, i;
	uint8_t void_mask;
	unsigned int chunk_index, block_index;
	bool in_foreground, player_blocked;
	ray_t cam_ray;
	block_t *block;
	list_t *bucket;
	v3i block_loc, player_loc;
	v3i min_block, max_block;
	list_t *shadows[world->block_size];

	// player_loc + raycasting for foregrounding vars
	in_foreground = false;
	player_loc = v3i_from_v3d(world->player->ray.pos);

	cam_ray = (ray_t){
		world->player->ray.pos,
		PLAYER_VIEW_DIR
	};
	cam_ray.pos.z += world->player->size.z / 2;
	player_blocked = raycast_to_block(world, cam_ray, NULL, NULL);
	
	// block range
	for (i = 0; i < 3; i++) {
		v3i_set(&min_block, i, MAX(0, v3i_get(&player_loc, i) - camera.render_dist));
		v3i_set(&max_block, i, MIN(world->block_size, v3i_get(&player_loc, i) + camera.render_dist));
	}

	// render targets
	SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0x00);
	SDL_SetRenderTarget(renderer, foreground);
	SDL_RenderClear(renderer);

	SDL_SetRenderDrawColor(renderer, BG_GRAY, BG_GRAY, BG_GRAY, 0xFF);
	SDL_SetRenderTarget(renderer, background);
	SDL_RenderClear(renderer);

	// gen shadows
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, SHADOW_ALPHA);
	render_generate_shadows(world, &shadows);

	for (z = min_block.z; z < max_block.z; z++) {
		// render shadows
		if (shadows[z] != NULL)
			for (i = 0; i < shadows[z]->size; i++)
				render_iso_circle(*(circle_t *)shadows[z]->items[i]);

		// change to foreground when ready
		if (player_blocked && !in_foreground && z > player_loc.z) {
			// render transparent textures on z layer above foreground to background
			for (y = min_block.y; y < max_block.y; y++) {
				for (x = min_block.x; x < max_block.x; x++) {
					block_loc = (v3i){x, y, z};
					chunk_block_indices(world, block_loc, &chunk_index, &block_index);

					if ((block = world->chunks[chunk_index]->blocks[block_index]) != NULL
					  && block->texture->transparent) {
						render_block(world, block, block_loc, void_mask);
					}
				}
			}

			SDL_SetRenderTarget(renderer, foreground);
			in_foreground = true;
		}

		// render blocks and buckets
		for (y = min_block.y; y < max_block.y; y++) {
			for (x = min_block.x; x < max_block.x; x++) {
				block_loc = (v3i){x, y, z};
				chunk_block_indices(world, block_loc, &chunk_index, &block_index);
				
				// blocks
				if ((block = world->chunks[chunk_index]->blocks[block_index]) != NULL) {
					if (block->texture->type == TEX_VOXEL) {
						void_mask = 0x0;

						for (i = 0; i < 3; i++) {
							void_mask <<= 1;
							void_mask |= (v3i_get(&block_loc, i) == v3i_get(&max_block, i) - 1 ? 0x1 : 0x0);
						}

						void_mask |= (block_loc.z == player_loc.z && !(block->expose_mask & 0x1) ? 0x1 : 0x0);
					}

					render_block(world, block, block_loc, void_mask);
				}

				// entities
				if ((bucket = world->chunks[chunk_index]->buckets[block_index]) != NULL)
					for (i = 0; i < bucket->size; i++)
						render_entity(bucket->items[i]);
			}
		}
	}	

	// render target surfaces to window
	if (in_foreground) { // target will be foreground if true
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
		render_iso_circle(camera.view_circle);
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	}

	SDL_SetRenderTarget(renderer, NULL);
	SDL_RenderCopy(renderer, background, &camera.viewport, NULL);
	SDL_RenderCopy(renderer, foreground, &camera.viewport, NULL);

	// destroy shadows
	for (z = 0; z < world->block_size; z++)
		if (shadows[z] != NULL)
			list_deep_destroy(shadows[z]);
}
