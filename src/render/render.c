#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "render.h"
#include "textures.h"
#include "primitives.h"
#include "gui.h"
#include "../camera.h"
#include "../data_structures/array.h"
#include "../data_structures/hashmap.h"
#include "../data_structures/list.h"
#include "../textures.h"
#include "../vector.h"
#include "../raycast.h"
#include "../utils.h"
#include "../world.h"
#include "../world_masks.h"
#include "../world_bucket.h"

#define BG_GRAY 31
#define SHADOW_ALPHA 63

const int VOXEL_Z_HEIGHT = VOXEL_HEIGHT - (VOXEL_WIDTH >> 1);
const v3d PLAYER_VIEW_DIR = {VOXEL_HEIGHT, VOXEL_HEIGHT, VOXEL_WIDTH};

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
	render_textures_init();
}

void render_quit() {
	// foreground and background are free'd by DestroyRenderer
	SDL_DestroyRenderer(renderer);
	renderer = NULL;
	foreground = NULL;
	background = NULL;
}

void render_info_gen_shadows(render_info_t *info, world_t *world) {
	size_t i, arr_idx;
	entity_t *entity;
	block_t *block;
	circle_t *shadow;
	v3d shadow_pos;
	v3i shadow_loc;

	info->shadows = malloc(sizeof(array_t *) * info->z_levels);

	// generate *shadows and z sort
	for (i = 0; i < info->z_levels; i++)
		info->shadows[i] = NULL;

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

		shadow_pos.z = shadow_loc.z + 1;
		arr_idx = shadow_loc.z - camera.rndr_start.z;

		if (arr_idx < info->z_levels) {
			shadow = malloc(sizeof(circle_t));
			shadow->loc = project_v3d(shadow_pos);
			shadow->radius = entity->sprites[0]->tex.sprite->size.x >> 1;

			if (info->shadows[arr_idx] == NULL)
				info->shadows[arr_idx] = array_create(2);

			array_add(info->shadows[arr_idx], shadow);
		}
	}
}

// generates entity->num_sprites packets
render_packet_t **render_gen_entity_packets(entity_t *entity) {
	v3d entity_pos;
	v2i screen_pos;
	render_packet_t **packets;

	entity_pos = entity->ray.pos;
	entity_pos.z -= entity->size.z / 2;
	screen_pos = project_v3d(entity_pos);

	packets = malloc(sizeof(render_packet_t *) * entity->num_sprites);

	for (size_t i = 0; i < entity->num_sprites; ++i) {
		packets[i] = malloc(sizeof(render_packet_t));

		packets[i]->pos = screen_pos;
		packets[i]->texture = entity->sprites[i];
		packets[i]->state.anim = entity->anim_states[i];
	}

	return packets;
}

v2i render_block_project(v3i loc) {
	// modify loc so that it is the back center corner of voxel from camera perspective
	switch (camera.rotation) {
		case 1:
			++loc.x;
			break;
		case 2:
			++loc.x;
			++loc.y;
			break;
		case 3:
			++loc.y;
			break;
	}

	return project_v3i(loc);
}

void render_info_add_entity(array_t *packet_arr, entity_t *entity) {
	if (entity->num_sprites) {
		render_packet_t **packets = render_gen_entity_packets(entity);

		for (int i = 0; i < entity->num_sprites; ++i)
			array_add(packet_arr, packets[i]);

		free(packets);
	}
}

void render_info_add_block(array_t *packet_arr, world_t *world, block_t *block, v3i loc) {
	render_packet_t *packet = NULL;

	if (block->texture->type == TEX_VOXEL) {
		packet = malloc(sizeof(render_packet_t));

		packet->pos = render_block_project(loc);
		packet->texture = block->texture;
		packet->state.voxel_masks = world_voxel_masks(block, loc);
	} else if (world_exposed(block)) {
		packet = malloc(sizeof(render_packet_t));

		packet->pos = render_block_project(loc);
		packet->texture = block->texture;

		if (block->texture->type == TEX_CONNECTED)
			packet->state.connected_mask = world_connected_mask(block);
		else
			packet->state.tex = block->tex_state;
	}

	if (packet != NULL)
		array_add(packet_arr, packet);
}

render_info_t *render_gen_info(world_t *world) {
	int i;
	double block_y;
	ray_t cam_ray;
	block_t *block;
	list_t *bucket;
	list_node_t *bucket_trav;
	v3i loc;
	render_info_t *info;
	array_t *level;

	cam_ray = (ray_t){
		world->player->ray.pos,
		camera_reverse_rotated_v3d(PLAYER_VIEW_DIR)
	};
	cam_ray.pos.z += world->player->size.z / 2;

	info = malloc(sizeof(render_info_t));

	info->z_levels = camera.rndr_end.z - camera.rndr_start.z + 1;
	info->packets = malloc(sizeof(array_t *) * info->z_levels);

	for (i = 0; i < info->z_levels; ++i)
		info->packets[i] = array_create((camera.rndr_dist + 1) * (camera.rndr_dist + 1));

	if (raycast_to_block(world, cam_ray, raycast_block_exists, NULL, NULL))
		info->z_split = (int)camera.pos.z - camera.rndr_start.z;
	else
		info->z_split = 0;

	info->cam_viewport = camera.viewport;

	render_info_gen_shadows(info, world);

	for (i = 0; i < info->z_levels; ++i) {
		level = info->packets[i];
		loc.z = camera.rndr_start.z + i;

		// render blocks and buckets
		loc.y = camera.rndr_start.y;
		for (; loc.y != camera.rndr_end.y + camera.rndr_inc.y; loc.y += camera.rndr_inc.y) {
			loc.x = camera.rndr_start.x;
			for (; loc.x != camera.rndr_end.x + camera.rndr_inc.x; loc.x += camera.rndr_inc.x) {
				world_get_render_loc(world, loc, &block, &bucket);

				if (block != NULL) {
					if (block->texture->transparent && bucket != NULL) { // draw block sorted between entities
						block_y = ((double)loc.x + 0.5) * camera.rndr_inc.x
							    + ((double)loc.y + 0.5) * camera.rndr_inc.y;
						bucket_trav = bucket->root;

						while (bucket_trav != NULL && world_bucket_y(bucket_trav->item) < block_y) {
							render_info_add_entity(level, bucket_trav->item);
							bucket_trav = bucket_trav->next;
						}

						render_info_add_block(level, world, block, loc);
						
						while (bucket_trav != NULL) {
							render_info_add_entity(level, bucket_trav->item);
							bucket_trav = bucket_trav->next;
						}
					} else { // draw entities over block regardless
						render_info_add_block(level, world, block, loc);

						if (bucket != NULL)
							LIST_FOREACH(bucket_trav, bucket)
								render_info_add_entity(level, bucket_trav->item);
					}
				} else if (bucket != NULL) {
					LIST_FOREACH(bucket_trav, bucket)
						render_info_add_entity(level, bucket_trav->item);
				}
			}
		}
	}

	return info;
}

// also destroys info
void render_from_info(render_info_t *info) {
	size_t i, j;

	SDL_SetRenderTarget(renderer, background);
	SDL_SetRenderDrawColor(renderer, BG_GRAY, BG_GRAY, BG_GRAY, 0xFF); // background
	SDL_RenderClear(renderer);

	for (i = 0; i < info->z_levels; ++i) {
		// packets
		SDL_SetRenderDrawColor(renderer, BG_GRAY, BG_GRAY, BG_GRAY, 0x7F); // outline

		for (j = 0; j < info->packets[i]->size; ++j)
			render_render_packet(info->packets[i]->items[j]);

		array_destroy(info->packets[i], true);

		// shadows
		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, SHADOW_ALPHA);
	
		if (info->shadows[i] != NULL) {
			for (j = 0; j < info->shadows[i]->size; ++j)
				render_iso_circle(*(circle_t *)info->shadows[i]->items[j]);

			array_destroy(info->shadows[i], true);
		}

		if (info->z_split && i == info->z_split) {
			SDL_SetRenderTarget(renderer, foreground);
			SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0x00);
			SDL_RenderClear(renderer);
		}
	}

	free(info->packets);
	free(info->shadows);

	if (info->z_split) {
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
		SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0x00);
		render_iso_circle(camera.view_circle);
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	}

	SDL_SetRenderTarget(renderer, NULL);
	SDL_RenderCopy(renderer, background, &info->cam_viewport, NULL);

	if (info->z_split)
		SDL_RenderCopy(renderer, foreground, &info->cam_viewport, NULL);

	free(info);
}
