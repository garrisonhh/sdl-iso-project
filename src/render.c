#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "render.h"
#include "render/gui.h"
#include "world.h"
#include "world/masks.h"
#include "world/bucket.h"
#include "lib/vector.h"
#include "lib/utils.h"
#include "lib/array.h"
#include "lib/hashmap.h"
#include "lib/list.h"
#include "camera.h"
#include "player.h"
#include "textures.h"
#include "raycast.h"

SDL_Renderer *RENDERER;
SDL_Texture *FOREGROUND, *BACKGROUND;

void render_init(SDL_Window *window) {
	RENDERER = SDL_CreateRenderer(window, -1,
								  SDL_RENDERER_ACCELERATED
								  | SDL_RENDERER_PRESENTVSYNC
								  | SDL_RENDERER_TARGETTEXTURE);
	if (RENDERER == NULL) {
		printf("unable to create RENDERER:\n%s\n", SDL_GetError());
		exit(1);
	}

	SDL_SetRenderDrawBlendMode(RENDERER, SDL_BLENDMODE_BLEND);
	SDL_RenderSetIntegerScale(RENDERER, true);

	FOREGROUND = SDL_CreateTexture(RENDERER,
								   RENDER_FORMAT,
								   SDL_TEXTUREACCESS_TARGET,
								   SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetTextureBlendMode(FOREGROUND, SDL_BLENDMODE_BLEND);

	BACKGROUND = SDL_CreateTexture(RENDERER,
								   RENDER_FORMAT,
								   SDL_TEXTUREACCESS_TARGET,
								   SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetTextureBlendMode(BACKGROUND, SDL_BLENDMODE_BLEND);
}

// call from game.c
void render_game_init() {
	camera_init();
}

void render_quit() {
	// FOREGROUND and BACKGROUND are free'd by DestroyRenderer
	SDL_DestroyRenderer(RENDERER);
	RENDERER = NULL;
	FOREGROUND = NULL;
	BACKGROUND = NULL;
}

void render_info_add_shadows(render_info_t *info, world_t *world) {
	v3d shadow_pos;
	v3i shadow_loc;
	entity_t *entity;
	list_node_t *node;
	block_t *block;
	render_packet_t *packet;

	LIST_FOREACH(node, world->entities) {
		entity = node->item;
		shadow_pos = entity->data.ray.pos;
		shadow_loc = v3i_from_v3d(shadow_pos);

		while (shadow_loc.z >= 0) {
			if ((block = world_get(world, shadow_loc)) != NULL
			 && !block->texture->transparent) {
				break;
			}
			--shadow_loc.z;
		}

		++shadow_loc.z;

		shadow_pos.z = shadow_loc.z;
		packet = render_shadow_packet_create(shadow_loc, project_v3d(shadow_pos),
											 entity->data.sprite->size.x >> 1);

		if (info->cam_hit && shadow_loc.z > info->z_split)
			array_push(info->fg_packets, packet);
		else
			array_push(info->bg_packets, packet);
	}
}

// TODO I think this is the sticking point, try optimizing render packet sorting so this
// function can be eliminated completely if possible
void render_info_add_packets_at(array_t *packets, world_t *world, v3i loc) {
	double block_y;
	block_t *block;
	list_t *bucket;
	list_node_t *bucket_trav;

	world_get_render_loc(world, loc, &block, &bucket);

	if (block != NULL) {
		if (block->texture->transparent && bucket != NULL) { // draw block sorted between entities
			block_y = -(((double)loc.x + 0.5) * camera.facing.x
					  + ((double)loc.y + 0.5) * camera.facing.y);
			bucket_trav = bucket->root;

			while (bucket_trav != NULL && world_bucket_y(bucket_trav->item) < block_y) {
				entity_add_render_info(packets, bucket_trav->item);
				bucket_trav = bucket_trav->next;
			}

			block_add_render_info(packets, block, loc);

			while (bucket_trav != NULL) {
				entity_add_render_info(packets, bucket_trav->item);
				bucket_trav = bucket_trav->next;
			}
		} else { // draw entities over block regardless
			block_add_render_info(packets, block, loc);

			if (bucket != NULL)
				LIST_FOREACH(bucket_trav, bucket)
					entity_add_render_info(packets, bucket_trav->item);
		}
	} else if (bucket != NULL) {
		LIST_FOREACH(bucket_trav, bucket)
			entity_add_render_info(packets, bucket_trav->item);
	}
}

void render_info_single_ray(array_t *packets, world_t *world, v3i loc, int min_z) {
	int i;

	while (loc.z >= min_z) {
		render_info_add_packets_at(packets, world, loc);

		loc = v3i_add(loc, camera.facing);

		// check out of bounds
		for (i = 0; i < 3; ++i)
			if ((v3i_IDX(camera.facing, i) < 0 && v3i_IDX(loc, i) < 0)
			 || (v3i_IDX(camera.facing, i) > 0 && v3i_IDX(loc, i) >= world->block_size))
				return;
	}
}

// if max_z is negative, uses camera-defined max_z
void render_info_voxel_raycast(array_t *packets, world_t *world, int max_z, int min_z) {
	const v3i limits = camera.render_limits;

	size_t num_locs = ((limits.x + 1) * (limits.y + 1))
					+ ((limits.x + 1) * limits.z)
					+ (limits.y * limits.z);
	v3i locs[num_locs];
	int i, index = 0;
	v3i offset;

	// x-y
	offset.z = 0;

	for (offset.x = 0; offset.x <= limits.x; ++offset.x)
		for (offset.y = 0; offset.y <= limits.y; ++offset.y)
			locs[index++] = offset;

	// x-z
	offset.y = 0;

	for (offset.x = 0; offset.x <= limits.x; ++offset.x)
		for (offset.z = 1; offset.z <= limits.z; ++offset.z)
			locs[index++] = offset;

	// y-z
	offset.x = 0;

	for (offset.y = 1; offset.y <= limits.y; ++offset.y)
		for (offset.z = 1; offset.z <= limits.z; ++offset.z)
			locs[index++] = offset;

	for (i = 0; i < num_locs; ++i) {
		// adjust value
		locs[i] = v3i_sub(camera.render_center, camera_reverse_rotated_v3i(locs[i]));

		if (!(max_z < 0) && locs[i].z > max_z)
			locs[i] = v3i_add(locs[i], v3i_scalei(camera.facing, locs[i].z - max_z));

		// raycast
		render_info_single_ray(packets, world, locs[i], min_z);
	}
}

render_info_t *render_gen_info(world_t *world) {
	ray_t cam_ray;
	render_info_t *info;

	info = malloc(sizeof(render_info_t));

	// camera
	cam_ray = (ray_t){player_get_head_pos(), v3d_scale(camera.view_dir, -1.0)};

	info->cam_hit = raycast_to_block(world, cam_ray, raycast_block_exists, NULL, NULL);
	info->cam_viewport = camera.viewport;

	// packets
	info->bg_packets = array_create(256);
	info->fg_packets = (info->cam_hit ? array_create(256) : NULL);
	info->z_split = (info->cam_hit ? (int)camera.pos.z : -1);

	camera_update_limits(world->block_size);

	render_info_add_shadows(info, world);

	if (info->cam_hit) {
		render_info_voxel_raycast(info->fg_packets, world, -1, info->z_split + 1);
		render_info_voxel_raycast(info->bg_packets, world, info->z_split, 0);
	} else {
		render_info_voxel_raycast(info->bg_packets, world, -1, 0);
	}

	// sort packets
	array_qsort(info->bg_packets, render_packet_compare);

	if (info->cam_hit)
		array_qsort(info->fg_packets, render_packet_compare);

	return info;
}

void render_info_destroy(render_info_t *info) {
	array_destroy(info->bg_packets, true);

	if (info->cam_hit)
		array_destroy(info->fg_packets, true);

	free(info);
}

void render_from_info(render_info_t *info) {
	size_t i;

	SDL_SetRenderTarget(RENDERER, BACKGROUND);
	SDL_SetRenderDrawColor(RENDERER, BG_GRAY, BG_GRAY, BG_GRAY, 0xFF); // BACKGROUND
	SDL_RenderClear(RENDERER);

	for (i = 0; i < info->bg_packets->size; ++i)
		render_from_packet(info->bg_packets->items[i]);

	if (info->cam_hit) {
		SDL_SetRenderTarget(RENDERER, FOREGROUND);
		SDL_SetRenderDrawColor(RENDERER, 0xFF, 0x00, 0x00, 0x00);
		SDL_RenderClear(RENDERER);

		for (i = 0; i < info->fg_packets->size; ++i)
			render_from_packet(info->fg_packets->items[i]);

		SDL_SetRenderDrawBlendMode(RENDERER, SDL_BLENDMODE_NONE);
		SDL_SetRenderDrawColor(RENDERER, 0xFF, 0x00, 0x00, 0x00);
		render_iso_circle(camera.view_circle);
		SDL_SetRenderDrawBlendMode(RENDERER, SDL_BLENDMODE_BLEND);
	}

	SDL_SetRenderTarget(RENDERER, NULL);
	SDL_RenderCopy(RENDERER, BACKGROUND, &info->cam_viewport, NULL);

	if (info->cam_hit)
		SDL_RenderCopy(RENDERER, FOREGROUND, &info->cam_viewport, NULL);
}
