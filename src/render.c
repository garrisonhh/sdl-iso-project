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

const v3d CAMERA_VIEW_DIR = {VOXEL_WIDTH >> 1, VOXEL_WIDTH >> 1, VOXEL_Z_HEIGHT};
const int VRAY_Z_PER_BLOCK = (VOXEL_WIDTH >> 1) / (VOXEL_Z_HEIGHT - (VOXEL_WIDTH >> 1));

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
	SDL_RenderSetIntegerScale(renderer, true);

	foreground = SDL_CreateTexture(renderer,
								   RENDER_FORMAT,
								   SDL_TEXTUREACCESS_TARGET,
								   SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetTextureBlendMode(foreground, SDL_BLENDMODE_BLEND);

	background = SDL_CreateTexture(renderer,
								   RENDER_FORMAT,
								   SDL_TEXTUREACCESS_TARGET,
								   SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetTextureBlendMode(background, SDL_BLENDMODE_BLEND);

	camera_init();
}

void render_quit() {
	// foreground and background are free'd by DestroyRenderer
	SDL_DestroyRenderer(renderer);
	renderer = NULL;
	foreground = NULL;
	background = NULL;
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
	// TODO check loc exiting world bounds
	int i = 0;

	while (loc.z >= min_z) {
		render_info_add_packets_at(packets, world, loc);
		loc = v3i_add(loc, camera.facing);

		// z offset
		if (i++ == VRAY_Z_PER_BLOCK) {
			loc.x += camera.facing.x;
			loc.y += camera.facing.y;
			i = 0;
		}
	}
}

void render_info_voxel_raycast(array_t *packets, world_t *world, v3i center, int max_z, int min_z) {
	v3i offset;
	int col_start, col_end;
	int i, j;
	int center_z_offset;

	offset = (v3i){0, 0, 0};

	center_z_offset = max_z - center.z;
	center = v3i_sub(center, v3i_scalei(camera.facing, center_z_offset));

	center.x -= camera.facing.x * (center_z_offset / VRAY_Z_PER_BLOCK);
	center.y -= camera.facing.y * (center_z_offset / VRAY_Z_PER_BLOCK);

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
	v3i loc;
	render_info_t *info;

	info = malloc(sizeof(render_info_t));

	// camera
	info->cam_hit = false;
	cam_ray = (ray_t){
		camera.pos,
		camera_reverse_rotated_v3d(CAMERA_VIEW_DIR)
	};
	cam_ray.pos.z += 0.5;

	info->cam_hit = raycast_to_block(world, cam_ray, raycast_block_exists, NULL, NULL);
	info->cam_viewport = camera.viewport;

	// packets
	info->bg_packets = array_create(256);

	loc = v3i_from_v3d(camera.pos);

	if (info->cam_hit) {
		info->z_split = (int)camera.pos.z;
		info->fg_packets = array_create(256);

		render_info_voxel_raycast(info->fg_packets, world, loc, world->block_size - 1, info->z_split);
		render_info_voxel_raycast(info->bg_packets, world, loc, info->z_split, 0);
	} else {
		info->z_split = -1;
		info->fg_packets = NULL;

		render_info_voxel_raycast(info->bg_packets, world, loc, world->block_size - 1, 0);
	}

	render_info_add_shadows(info, world);

	// sort
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

	SDL_SetRenderTarget(renderer, background);
	SDL_SetRenderDrawColor(renderer, BG_GRAY, BG_GRAY, BG_GRAY, 0xFF); // background
	SDL_RenderClear(renderer);

	for (i = 0; i < info->bg_packets->size; ++i)
		render_from_packet(info->bg_packets->items[i]);

	if (info->cam_hit) {
		SDL_SetRenderTarget(renderer, foreground);
		SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0x00);
		SDL_RenderClear(renderer);

		for (i = 0; i < info->fg_packets->size; ++i)
			render_from_packet(info->fg_packets->items[i]);

		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
		SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0x00);
		render_iso_circle(camera.view_circle);
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	}

	SDL_SetRenderTarget(renderer, NULL);
	SDL_RenderCopy(renderer, background, &info->cam_viewport, NULL);

	if (info->cam_hit)
		SDL_RenderCopy(renderer, foreground, &info->cam_viewport, NULL);
}
