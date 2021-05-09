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
#include "data_structures/array.h"
#include "data_structures/hashmap.h"
#include "data_structures/list.h"
#include "camera.h"
#include "render_primitives.h"
#include "render_textures.h"
#include "utils.h"
#include "world.h"

#define BG_GRAY 31
#define SHADOW_ALPHA 63

const int VOXEL_Z_HEIGHT = VOXEL_HEIGHT - (VOXEL_WIDTH >> 1);
const v3d PLAYER_VIEW_DIR = {VOXEL_HEIGHT, VOXEL_HEIGHT, VOXEL_WIDTH};

const v2i OUTLINE_TOP_EDGES[][2] = {
	{
		(v2i){0, (VOXEL_WIDTH >> 1) - 1},
		(v2i){VOXEL_WIDTH >> 1, (VOXEL_WIDTH >> 2) - 1},
	},
	{
		(v2i){-(VOXEL_WIDTH >> 1), (VOXEL_WIDTH >> 2) - 1},
		(v2i){0, 0},
	},
	{
		(v2i){-(VOXEL_WIDTH >> 1), (VOXEL_WIDTH >> 2)},
		(v2i){0, (VOXEL_WIDTH >> 1)},
	},
	{
		(v2i){0, 0},
		(v2i){VOXEL_WIDTH >> 1, (VOXEL_WIDTH >> 2) - 1},
	},
};
const v2i OUTLINE_CORNERS[] = {
	(v2i){(VOXEL_WIDTH >> 1) - 1, (VOXEL_WIDTH >> 2)},
	(v2i){-(VOXEL_WIDTH >> 1), (VOXEL_WIDTH >> 2)},
};

SDL_Renderer *renderer;
SDL_Texture *foreground, *background;

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
	SDL_SetTextureBlendMode(background, SDL_BLENDMODE_BLEND);

	camera_init();
	render_textures_init();
}

void render_destroy() {
	// foreground and background are free'd by DestroyRenderer
	SDL_DestroyRenderer(renderer);
	renderer = NULL;
	foreground = NULL;
	background = NULL;
}

void render_generate_shadows(world_t *world, array_t *(*shadows)[world->block_size]) {
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
			if ((block = world_get(world, shadow_loc)) != NULL
			 && !block->texture->transparent) {
				break;
			}
			shadow_loc.z--;
		}

		shadow_loc.z++;
		shadow_pos.z = shadow_loc.z;

		if (shadow_loc.z >= 0 && shadow_loc.z < world->block_size) {
			shadow = malloc(sizeof(circle_t));
			shadow->loc = project_v3d(shadow_pos, true);
			shadow->radius = (int)((entity->size.x * VOXEL_WIDTH) / 2.0);

			if ((*shadows)[shadow_loc.z] == NULL)
				(*shadows)[shadow_loc.z] = array_create(2);

			array_add((*shadows)[shadow_loc.z], shadow);
		}
	}
}

void render_entity(entity_t *entity) {
	v3d entity_pos;
	v2i screen_pos;

	entity_pos = entity->ray.pos;
	entity_pos.z -= entity->size.z / 2;
	screen_pos = project_v3d(entity_pos, true);

	render_sprite(entity->sprite, screen_pos, entity->anim_cell);
}

unsigned render_find_void_mask(v3i loc, v3i max_block, int player_z, unsigned block_expose_mask) {
	int i;
	unsigned void_mask;

	void_mask = 0x0;

	// world borders
	for (i = 0; i < 3; ++i)
		if (v3i_get(&loc, i) == v3i_get(&max_block, i) - 1)
			void_mask |= 0x1 << i;

	// foreground/background "fog of war"
	if (loc.z == player_z && !(block_expose_mask & 0x7))
		void_mask |= 0x7;

	return void_mask;
}

void render_block_outline(v3i loc, unsigned outline_mask, unsigned expose_mask) {
	int i, j;
	v2i block_pos;
	v2i pos;

	// bottom edges
	block_pos = project_v3i(loc, true);

	for (i = 0; i <= 1; ++i) {
		if ((outline_mask >> (i + 6)) & 1 && (expose_mask >> i) & 1) {
			render_aligned_line(v2i_add(block_pos, OUTLINE_TOP_EDGES[i << 1][0]),
								v2i_add(block_pos, OUTLINE_TOP_EDGES[i << 1][1]));
		}
	}

	// top edges
	block_pos.y -= VOXEL_Z_HEIGHT;

	if (expose_mask >> 2) {
		for (i = 0; i < 4; ++i) {
			if ((outline_mask >> i) & 1) {
				render_aligned_line(v2i_add(block_pos, OUTLINE_TOP_EDGES[i][0]),
									v2i_add(block_pos, OUTLINE_TOP_EDGES[i][1]));
			}
		}
	}

	// corners
	for (i = 0; i <= 1; ++i) {
		if ((outline_mask >> (i + 4)) & 1 && (expose_mask >> i) & 1) {
			pos = v2i_add(block_pos, OUTLINE_CORNERS[i]);

			for (j = 1; j <= VOXEL_Z_HEIGHT - ((outline_mask >> (i + 6)) & 1); ++j)
				SDL_RenderDrawPoint(renderer, pos.x, pos.y + j);
		}
	}
}

void render_block(world_t *world, block_t *block, v3i loc, unsigned void_mask) {
	texture_type_e tex_type = block->texture->type;

	if (tex_type == TEX_VOXEL) {
		if (block->expose_mask || void_mask) {
			unsigned outline_mask;

			render_voxel_texture(block->texture->tex.voxel,
								 project_v3i(loc, true),
								 block->expose_mask, void_mask);

			if ((outline_mask = block->tex_state.outline_mask))
				render_block_outline(loc, outline_mask, block->expose_mask & ~void_mask);
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
										 block->tex_state.connected_mask);
				break;
			case TEX_SHEET:
				render_sheet_texture(block->texture->tex.sheet,
									 project_v3i(loc, true),
									 block->tex_state.cell);
				break;
			default:
				break;
		}
	}
}

void render_world(world_t *world) {
	int i;
	unsigned chunk_index, block_index;
	unsigned void_mask;
	bool render_to_fg, player_blocked;
	ray_t cam_ray;
	chunk_t *chunk;
	block_t *block;
	list_t *bucket;
	list_node_t *bucket_trav;
	v3i loc, player_loc;
	v3i min_block, max_block;
	array_t *shadows[world->block_size];

	// player_loc + raycasting for foregrounding
	render_to_fg = false;
	player_loc = v3i_from_v3d(world->player->ray.pos);

	cam_ray = (ray_t){
		world->player->ray.pos,
		PLAYER_VIEW_DIR
	};
	cam_ray.pos.z += world->player->size.z / 2;
	player_blocked = raycast_to_block(world, cam_ray, raycast_block_exists, NULL, NULL);

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

	// shadow setup
	render_generate_shadows(world, &shadows);

	for (loc.z = min_block.z; loc.z < max_block.z; loc.z++) {
		// render shadows
		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, SHADOW_ALPHA);

		if (shadows[loc.z] != NULL)
			for (i = 0; i < shadows[loc.z]->size; i++)
				render_iso_circle(*(circle_t *)shadows[loc.z]->items[i]);

		// change to foreground when ready
		if (player_blocked && !render_to_fg && loc.z > player_loc.z) {
			SDL_SetRenderTarget(renderer, foreground);
			render_to_fg = true;
		}

		// render blocks and buckets
		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x3F); // block outlines
		
		for (loc.y = min_block.y; loc.y < max_block.y; loc.y++) {
			for (loc.x = min_block.x; loc.x < max_block.x; loc.x++) {
				world_indices(world, loc, &chunk_index, &block_index);

				if ((chunk = world->chunks[chunk_index]) != NULL) {
					if ((block = chunk->blocks[block_index]) != NULL) {
						if (block->texture->type == TEX_VOXEL) {
							void_mask = render_find_void_mask(loc, max_block,
															  player_loc.z, block->expose_mask);
							
						}

						render_block(world, block, loc, void_mask);
					}

					if ((bucket = chunk->buckets[block_index]) != NULL)
						LIST_FOREACH(bucket_trav, bucket)
							render_entity(bucket_trav->item);
				}
			}
		}
	}	

	// render target surfaces to window
	if (render_to_fg) {
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
		render_iso_circle(camera.view_circle);
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	}

	SDL_SetRenderTarget(renderer, NULL);
	SDL_RenderCopy(renderer, background, &camera.viewport, NULL);
	SDL_RenderCopy(renderer, foreground, &camera.viewport, NULL);

	// destroy shadows
	for (loc.z = 0; loc.z < world->block_size; loc.z++)
		if (shadows[loc.z] != NULL)
			array_destroy(shadows[loc.z], true);
}
