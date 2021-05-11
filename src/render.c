#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "render.h"
#include "textures.h"
#include "gui.h"
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

// outline lines
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
								   SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetTextureBlendMode(foreground, SDL_BLENDMODE_BLEND);

	background = SDL_CreateTexture(renderer,
								   SDL_PIXELFORMAT_RGBA8888,
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
			shadow->loc = project_v3d(shadow_pos);
			shadow->radius = entity->sprites[0]->size.x >> 1;

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
	screen_pos = project_v3d(entity_pos);

	for (size_t i = 0; i < entity->num_sprites; ++i)
		render_sprite(entity->sprites[i], screen_pos, entity->anim_states[i].cell);
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

/*
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
*/

void render_block(world_t *world, block_t *block, v3i loc, unsigned void_mask) {
	texture_type_e tex_type = block->texture->type;

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

	if (tex_type == TEX_VOXEL) {
		if (block->expose_mask || void_mask) {
			//unsigned outline_mask;

			/*
			render_voxel_texture(block->texture->tex.voxel, project_v3i(loc),
								 block->expose_mask, void_mask);
			*/
			// TODO update mask code for camera rotation
			render_voxel_texture(block->texture->tex.voxel, project_v3i(loc),
								 0xF, 0x0);

			/*
			if ((outline_mask = block->tex_state.outline_mask))
				render_block_outline(loc, outline_mask, block->expose_mask & ~void_mask);
			*/
		}
	} else if (block->expose_mask) {
		// the amount of times I had to type "tex" or "texture" here is hilarious lol
		switch (tex_type) {
			case TEX_TEXTURE:
				render_sdl_texture(block->texture->tex.texture, project_v3i(loc));
				break;
			case TEX_CONNECTED:
				render_connected_texture(block->texture->tex.connected, project_v3i(loc),
										 block->tex_state.connected_mask);
				break;
			case TEX_SHEET:
				render_sheet_texture(block->texture->tex.sheet, project_v3i(loc),
									 block->tex_state.cell);
				break;
			default:
				break;
		}
	}
}

void render_world(world_t *world) {
	int i;
	unsigned void_mask;
	bool render_to_fg, player_blocked;
	double block_y;
	ray_t cam_ray;
	block_t *block;
	list_t *bucket;
	list_node_t *bucket_trav;
	v3i loc, player_loc;
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

	// render targets
	SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0x00);
	SDL_SetRenderTarget(renderer, foreground);
	SDL_RenderClear(renderer);

	SDL_SetRenderDrawColor(renderer, BG_GRAY, BG_GRAY, BG_GRAY, 0xFF);
	SDL_SetRenderTarget(renderer, background);
	SDL_RenderClear(renderer);

	// shadow setup
	render_generate_shadows(world, &shadows);

	for (loc.z = camera.min_render.z; loc.z != camera.max_render.z; loc.z += camera.inc_render.z) {
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
		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x3F); // block outline color
		
		for (loc.y = camera.min_render.y; loc.y != camera.max_render.y; loc.y += camera.inc_render.y) {
			for (loc.x = camera.min_render.x; loc.x != camera.max_render.x; loc.x += camera.inc_render.x) {
				// world_indices(world, loc, &chunk_index, &block_index);

				world_get_render_loc(world, loc, &block, &bucket);

				if (block != NULL) {
					if (block->texture->type == TEX_VOXEL) {
						void_mask = render_find_void_mask(loc, camera.max_render,
														  player_loc.z, block->expose_mask);
						
					}

					if (block->texture->transparent) { // draw block sorted between entities
						if (bucket != NULL) {
							// TODO apply camera rotation to this
							// I think move entity_y and entity_bucket_compare to render.c, and then
							// calculate block_y with camera taken into account
							// also unsure what this means for entity sorting, it will have to be done
							// at least after every rotation

							block_y = (double)(loc.x + loc.y) + 1.0; // 1.0 for (0.5, 0.5) center of block
							bucket_trav = bucket->root;

							while (bucket_trav != NULL && entity_y(bucket_trav->item) < block_y) {
								render_entity(bucket_trav->item);
								bucket_trav = bucket_trav->next;
							}

							render_block(world, block, loc, void_mask);
							
							while (bucket_trav != NULL) {
								render_entity(bucket_trav->item);
								bucket_trav = bucket_trav->next;
							}
						} else {
							render_block(world, block, loc, void_mask);
						}
					} else { // draw entities over block regardless
						render_block(world, block, loc, void_mask);

						if (bucket != NULL)
							LIST_FOREACH(bucket_trav, bucket)
								render_entity(bucket_trav->item);
					}
				} else if (bucket != NULL) {
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
