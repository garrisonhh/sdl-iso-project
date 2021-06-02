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

// returns whether block has been hit for voxel raycasting
bool render_info_add_packets_at(array_t *packets, world_t *world, v3i loc) {
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

		return true;
	} else if (bucket != NULL) {
		LIST_FOREACH(bucket_trav, bucket)
			entity_add_render_info(packets, bucket_trav->item);
	}

	return false;
}

void render_info_single_ray(array_t *packets, world_t *world, v3i loc, int min_z) {
	int i;

	while (!render_info_add_packets_at(packets, world, loc)) {
		loc = v3i_add(loc, camera.facing);

		// check out of bounds
		for (i = 0; i < 3; ++i)
			if ((v3i_IDX(camera.facing, i) < 0 && v3i_IDX(loc, i) < 0)
			 || (v3i_IDX(camera.facing, i) > 0 && v3i_IDX(loc, i) >= world->block_size))
				return;
	}
}

void render_info_voxel_raycast(array_t *packets, world_t *world, v3i center, int max_z, int min_z) {
	v3i offset;
	int col_start, col_end;
	int i, j;
	int z_diff;

	offset = (v3i){0, 0, 0};

	z_diff = max_z - center.z;
	center = v3i_from_v3d(v3d_sub(v3d_from_v3i(center), v3d_scale(camera.view_dir, z_diff)));

	for (i = 0; i <= camera.vray_size; ++i) {
		col_start = abs(i - camera.vray_start);
		col_end = camera.vray_size - abs((camera.vray_size - i) - camera.vray_start);

		for (j = col_start; j <= col_end; ++j) {
			offset.x = i - camera.vray_middle;
			offset.y = j - camera.vray_middle;

			render_info_single_ray(packets, world, v3i_add(offset, center), min_z);
		}
	}
}

render_info_t *render_gen_info(world_t *world) {
	ray_t cam_ray;
	v3i cam_loc;
	render_info_t *info;

	info = malloc(sizeof(render_info_t));

	// camera
	cam_ray = (ray_t){camera.pos, (v3d){0, 0, 0}};
	cam_ray.dir = v3d_scale(camera.view_dir, -1.0);
	cam_ray.pos.z += 0.5;

	info->cam_hit = raycast_to_block(world, cam_ray, raycast_block_exists, NULL, NULL);
	info->cam_viewport = camera.viewport;

	// packets
	info->bg_packets = array_create(256);
	info->fg_packets = (info->cam_hit ? array_create(256) : NULL);

	render_info_add_shadows(info, world);

	cam_loc = v3i_from_v3d(camera.pos);

	if (info->cam_hit) {
		info->z_split = (int)camera.pos.z;

		render_info_voxel_raycast(info->fg_packets, world, cam_loc, world->block_size - 1, info->z_split);
		render_info_voxel_raycast(info->bg_packets, world, cam_loc, info->z_split, 0);
	} else {
		info->z_split = -1;

		render_info_voxel_raycast(info->bg_packets, world, cam_loc, world->block_size - 1, 0);
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
