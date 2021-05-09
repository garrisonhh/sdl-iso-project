#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>
#include "vector.h"
#include "content.h"
#include "render.h"
#include "textures.h"
#include "data_structures/hashmap.h"
#include "data_structures/hash_functions.h"

#define ANIMATION_FPS 12.0

texture_t **TEXTURES = NULL;
size_t NUM_TEXTURES;
hashmap_t *TEXTURE_MAP;

v2i VOXEL_DRAW_SIZE = {VOXEL_WIDTH, VOXEL_HEIGHT};

voxel_tex_t *VOID_VOXEL_TEXTURE;

SDL_Texture *load_sdl_texture(const char *path) {
	SDL_Texture *texture = NULL;
	SDL_Surface *loaded_surface = IMG_Load(path);

	if (loaded_surface == NULL) {
		printf("unable to load image %s:\n%s\n", path, IMG_GetError());
		exit(1);
	}

	texture = SDL_CreateTextureFromSurface(renderer, loaded_surface);
	
	if (texture == NULL) {
		printf("unable to create texture from %s:\n%s\n", path, SDL_GetError());
		exit(1);
	}

	SDL_FreeSurface(loaded_surface);
	return texture;
}

sprite_t *load_sprite(const char *path, json_object *obj) {
	sprite_t *sprite = malloc(sizeof(sprite_t));

	sprite->sheet = load_sdl_texture(path);

	if (content_has_key(obj, "sprite-size"))
		sprite->size = content_get_v2i(obj, "sprite-size");
	else
		SDL_QueryTexture(sprite->sheet, NULL, NULL, &sprite->size.x, &sprite->size.y);

	sprite->pos = (v2i){-(sprite->size.x >> 1), -sprite->size.y};

	if (content_has_key(obj, "anim-lengths")) {
		array_t *anim_len_objs = content_get_array(obj, "anim-lengths");

		sprite->num_anims = anim_len_objs->size;
		sprite->anim_lengths = malloc(sizeof(int) * anim_len_objs->size);

		for (size_t i = 0; i < anim_len_objs->size; ++i)
			sprite->anim_lengths[i] = json_object_get_int(anim_len_objs->items[i]);
	} else {
		sprite->num_anims = 1;
		sprite->anim_lengths = malloc(sizeof(int));

		sprite->anim_lengths[0] = 1;
	}

	return sprite;
}

// loads [path]_top.png and [path]_side.png
voxel_tex_t* load_voxel_texture(const char *path) {
	size_t len_path;
	char top_path[100], side_path[100];

	len_path = strlen(path) - 4;

	strncpy(top_path, path, len_path);
	strncpy(side_path, path, len_path);
	top_path[len_path] = 0;
	side_path[len_path] = 0;
	strcat(top_path, "_top.png");
	strcat(side_path, "_side.png");

	voxel_tex_t* voxel_tex = malloc(sizeof(voxel_tex_t));
	voxel_tex->top = load_sdl_texture(top_path);
	voxel_tex->side = load_sdl_texture(side_path);

	return voxel_tex;
}

/* TODO deprecated until connected tags are added. also rewrite this crap
connected_tex_t *load_connected_texture(const char *path) {
	char base_path[100];
	char top_path[100], bottom_path[100];
	char front_path[100], back_path[100];

	strcpy(base_path, path);
	strcpy(top_path, path);
	strcpy(bottom_path, path);
	strcpy(front_path, path);
	strcpy(back_path, path);

	strcat(base_path, "_base.png");
	strcat(top_path, "_top.png");
	strcat(bottom_path, "_bottom.png");
	strcat(front_path, "_front.png");
	strcat(back_path, "_back.png");

	connected_tex_t *connected_tex = malloc(sizeof(connected_tex_t));

	connected_tex->base = load_sdl_texture(base_path);
	connected_tex->top = load_sdl_texture(top_path);
	connected_tex->bottom = load_sdl_texture(bottom_path);
	connected_tex->front = load_sdl_texture(front_path);
	connected_tex->back = load_sdl_texture(back_path);

	return connected_tex;
}
*/

sheet_tex_t *load_sheet_texture(const char *path, json_object *obj) {
	sheet_tex_t *sheet_tex = malloc(sizeof(sheet_tex_t));
	v2i image_size, cell_size;
	
	sheet_tex->sheet = load_sdl_texture(path);

	if (content_has_key(obj, "cell-size"))
		cell_size = content_get_v2i(obj, "cell-size");
	else
		cell_size = VOXEL_DRAW_SIZE;

	SDL_QueryTexture(sheet_tex->sheet, NULL, NULL, &image_size.x, &image_size.y);

	sheet_tex->sheet_size = v2i_div(image_size, cell_size);

	return sheet_tex;
}

void entity_anim_swap(entity_t *entity, int anim) {
	entity->anim_cell.y = anim;
	entity->anim_state = 0.0;
}

void entity_sprite_tick(entity_t *entity, double time) {
	if (entity->sprite->anim_lengths[entity->anim_cell.y] > 1) {
		entity->anim_state += time * ANIMATION_FPS;

		if (entity->anim_state > entity->sprite->anim_lengths[entity->anim_cell.y])
			entity->anim_state -= (double)entity->sprite->anim_lengths[entity->anim_cell.y];

		entity->anim_cell.x = (int)entity->anim_state;
	}
}

void textures_load() {
	int i;

	// create tex_type map
	const int num_tex_types = 5;
	char *tex_type_strings[] = {
		"texture",
		"sprite",
		"voxel",
		"connected", // TODO connected tags
		"sheet",
	};
	texture_type_e *tex_type;
	hashmap_t *tex_type_map = hashmap_create(num_tex_types * 2, false, hash_string);

	for (i = 0; i < num_tex_types; ++i) {
		tex_type = malloc(sizeof(texture_type_e));
		*tex_type = (texture_type_e)i;
		hashmap_set(tex_type_map, tex_type_strings[i], strlen(tex_type_strings[i]), tex_type);
	}

	// json texture list
	json_object *file_obj;
	array_t *texture_objects;

	file_obj = content_load_file("assets/textures.json");
	texture_objects = content_get_array(file_obj, "textures");

	// set up globals
	NUM_TEXTURES = texture_objects->size;
	TEXTURES = malloc(sizeof(texture_t *) * NUM_TEXTURES);
	TEXTURE_MAP = hashmap_create(NUM_TEXTURES * 2, true, hash_string);

	// load textures
	json_object *texture_obj;
	const char *name, *tex_type_name, *rel_path;
	char file_path[80];
	bool transparent;
	size_t *texture_id;

	for (i = 0; i < NUM_TEXTURES; ++i) {
		TEXTURES[i] = malloc(sizeof(texture_t));

		texture_obj = texture_objects->items[i];

		name = content_get_string(texture_obj, "name");

		// type
		tex_type_name = content_get_string(texture_obj, "type");
		tex_type = hashmap_get(tex_type_map, (char *)tex_type_name, strlen(tex_type_name));

		if (tex_type == NULL) {
			printf("\"%s\" is an unrecognized texture type.\n", tex_type_name);
			exit(1);
		}

		// file path
		if (content_has_key(texture_obj, "path"))
			rel_path = content_get_string(texture_obj, "path");
		else
			rel_path = name;

		sprintf(file_path, "assets/%s.png", rel_path);

		// load texture
		switch (*tex_type) {
			case TEX_TEXTURE:
				TEXTURES[i]->tex.texture = load_sdl_texture(file_path);
				break;
			case TEX_SPRITE:
				TEXTURES[i]->tex.sprite = load_sprite(file_path, texture_obj);
				break;
			case TEX_VOXEL:
				TEXTURES[i]->tex.voxel = load_voxel_texture(file_path);
				break;
			case TEX_CONNECTED:
				printf("connected textures are not currently implemented.\n");
				exit(1);
				break;
			case TEX_SHEET:
				TEXTURES[i]->tex.sheet = load_sheet_texture(file_path, texture_obj);
				break;
		}

		// transparency
		if (*tex_type == TEX_VOXEL || *tex_type == TEX_SPRITE)
			transparent = false;
		else
			transparent = content_get_bool(texture_obj, "transparent");

		// save to array and hashmap
		TEXTURES[i]->type = *tex_type;
		TEXTURES[i]->transparent = transparent;

		texture_id = malloc(sizeof(size_t));
		*texture_id = i;
		hashmap_set(TEXTURE_MAP, (char *)name, strlen(name), texture_id);
	}

	// clean up and exit
	array_destroy(texture_objects, false);
	hashmap_destroy(tex_type_map, true);
	content_close_file(file_obj);

	VOID_VOXEL_TEXTURE = texture_from_key("void")->tex.voxel;
}

void textures_destroy() {
	for (size_t i = 0; i < NUM_TEXTURES; i++) {
		switch (TEXTURES[i]->type) {
			case TEX_TEXTURE:
				SDL_DestroyTexture(TEXTURES[i]->tex.texture);
				break;
			case TEX_SPRITE:
				SDL_DestroyTexture(TEXTURES[i]->tex.sprite->sheet);
				free(TEXTURES[i]->tex.sprite->anim_lengths);
				free(TEXTURES[i]->tex.sprite);
				break;
			case TEX_VOXEL:
				SDL_DestroyTexture(TEXTURES[i]->tex.voxel->top);
				SDL_DestroyTexture(TEXTURES[i]->tex.voxel->side);
				free(TEXTURES[i]->tex.voxel);
				break;
			case TEX_CONNECTED:
				SDL_DestroyTexture(TEXTURES[i]->tex.connected->base);
				SDL_DestroyTexture(TEXTURES[i]->tex.connected->top);
				SDL_DestroyTexture(TEXTURES[i]->tex.connected->bottom);
				SDL_DestroyTexture(TEXTURES[i]->tex.connected->front);
				SDL_DestroyTexture(TEXTURES[i]->tex.connected->back);
				free(TEXTURES[i]->tex.connected);
				break;
			case TEX_SHEET:
				SDL_DestroyTexture(TEXTURES[i]->tex.sheet->sheet);
				free(TEXTURES[i]->tex.sheet);
				break;
		}

		free(TEXTURES[i]);
	}

	VOID_VOXEL_TEXTURE = NULL;

	free(TEXTURES);
	TEXTURES = NULL;
	hashmap_destroy(TEXTURE_MAP, true);
	TEXTURE_MAP = NULL;
}

texture_t *texture_from_key(char *key) {
	size_t *value;

	if ((value = (size_t *)hashmap_get(TEXTURE_MAP, key, strlen(key))) == NULL) {
		printf("key not found in TEXTURE_MAP: %s\n", key);
		exit(1);
	}
	
	return TEXTURES[*value];
}

texture_state_t texture_state_from_type(texture_type_e tex_type) {
	texture_state_t tex_state;

	switch (tex_type) {
		case TEX_VOXEL:
			tex_state.outline_mask = 0x0;
			break;
		case TEX_CONNECTED:
			tex_state.connected_mask = 0x0;
			break;
		case TEX_SHEET:
			tex_state.cell = (v2i){0, 0};
			break;
		default:
			break;
	}

	return tex_state;
}
