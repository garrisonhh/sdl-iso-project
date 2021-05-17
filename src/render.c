#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "render.h"
#include "textures.h"
#include "render_primitives.h"
#include "render_textures.h"
#include "gui.h"
#include "vector.h"
#include "collision.h"
#include "raycast.h"
#include "data_structures/array.h"
#include "data_structures/hashmap.h"
#include "data_structures/list.h"
#include "camera.h"
#include "utils.h"
#include "world.h"
#include "world_masks.h"
#include "world_bucket.h"

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

array_t **render_generate_shadows(world_t *world) {
	int z, i;
	entity_t *entity;
	block_t *block;
	circle_t *shadow;
	v3d shadow_pos;
	v3i shadow_loc;
	array_t **shadows;

	shadows = malloc(sizeof(array_t *) * world->block_size);

	// generate *shadows and z sort
	for (z = 0; z < world->block_size; z++)
		shadows[z] = NULL;

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
			shadow->loc = project_v3d(shadow_pos);
			shadow->radius = entity->sprites[0]->tex.sprite->size.x >> 1;

			if (shadows[shadow_loc.z] == NULL)
				shadows[shadow_loc.z] = array_create(2);

			array_add(shadows[shadow_loc.z], shadow);
		}
	}

	return shadows;
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

	for (int i = 0; i < entity->num_sprites; ++i) {
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

// check result for NULL
render_packet_t *render_gen_block_packet(world_t *world, block_t *block, v3i loc) {
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
		packet->state.tex = block->tex_state;
	}

	return packet;
}

void render_info_add_entity(render_info_t *info, entity_t *entity) {
	if (entity->num_sprites) {
		render_packet_t **packets = render_gen_entity_packets(entity);

		for (int i = 0; i < entity->num_sprites; ++i)
			array_add(info->packets, packets[i]);

		free(packets);
	}
}

void render_info_add_block(render_info_t *info, world_t *world, block_t *block, v3i loc) {
	array_add(info->packets, render_gen_block_packet(world, block, loc));
}

render_info_t *render_gen_info(world_t *world) {
	double block_y;
	ray_t cam_ray;
	block_t *block;
	list_t *bucket;
	list_node_t *bucket_trav;
	v3i loc;
	render_info_t *info;

	cam_ray = (ray_t){
		world->player->ray.pos,
		camera_reverse_rotated_v3d(PLAYER_VIEW_DIR)
	};
	cam_ray.pos.z += world->player->size.z / 2;

	info = malloc(sizeof(render_info_t));
	info->packets = array_create(256); // TODO better estimate of num packets?
	info->camera_z = v3i_from_v3d(world->player->ray.pos).z;
	info->camera_blocked = raycast_to_block(world, cam_ray, raycast_block_exists, NULL, NULL);

	for (loc.z = camera.render_start.z; loc.z != camera.render_end.z + camera.render_inc.z; loc.z += camera.render_inc.z) {
		// render blocks and buckets
		for (loc.y = camera.render_start.y; loc.y != camera.render_end.y + camera.render_inc.y; loc.y += camera.render_inc.y) {
			for (loc.x = camera.render_start.x; loc.x != camera.render_end.x + camera.render_inc.x; loc.x += camera.render_inc.x) {
				world_get_render_loc(world, loc, &block, &bucket);

				if (block != NULL) {
					if (block->texture->transparent && bucket != NULL) { // draw block sorted between entities
						block_y = ((double)loc.x + 0.5) * camera.render_inc.x
							    + ((double)loc.y + 0.5) * camera.render_inc.y;
						bucket_trav = bucket->root;

						while (bucket_trav != NULL && world_bucket_y(bucket_trav->item) < block_y) {
							render_info_add_entity(info, bucket_trav->item);
							bucket_trav = bucket_trav->next;
						}

						render_info_add_block(info, world, block, loc);
						
						while (bucket_trav != NULL) {
							render_info_add_entity(info, bucket_trav->item);
							bucket_trav = bucket_trav->next;
						}
					} else { // draw entities over block regardless
						render_info_add_block(info, world, block, loc);

						if (bucket != NULL)
							LIST_FOREACH(bucket_trav, bucket)
								render_info_add_entity(info, bucket_trav->item);
					}
				} else if (bucket != NULL) {
					LIST_FOREACH(bucket_trav, bucket)
						render_info_add_entity(info, bucket_trav->item);
				}
			}
		}
	}

	return info;
}

// also destroys info
void render_from_info(render_info_t *info) {
	if (info != NULL) { // info will only be NULL on the very first frame
		size_t i;

		// TODO move this elsewhere?
		SDL_SetRenderDrawColor(renderer, BG_GRAY, BG_GRAY, BG_GRAY, 0xFF);
		SDL_RenderClear(renderer);

		for (i = 0; i < info->packets->size; ++i) {
			render_render_packet(info->packets->items[i]);
		}

		array_destroy(info->packets, true);
		free(info);
	}
}
