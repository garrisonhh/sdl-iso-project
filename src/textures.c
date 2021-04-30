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

sprite_t *load_sprite(const char *path) {
	sprite_t *sprite = (sprite_t *)malloc(sizeof(sprite_t));

	sprite->texture = load_sdl_texture(path);
	SDL_QueryTexture(sprite->texture, NULL, NULL, &sprite->size.x, &sprite->size.y);
	sprite->pos = (v2i){-(sprite->size.x >> 1), -sprite->size.y};

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

	voxel_tex_t* voxel_tex = (voxel_tex_t *)malloc(sizeof(voxel_tex_t));
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

	connected_tex_t *connected_tex = (connected_tex_t *)malloc(sizeof(connected_tex_t));

	connected_tex->base = load_sdl_texture(base_path);
	connected_tex->top = load_sdl_texture(top_path);
	connected_tex->bottom = load_sdl_texture(bottom_path);
	connected_tex->front = load_sdl_texture(front_path);
	connected_tex->back = load_sdl_texture(back_path);

	return connected_tex;
}
*/

sheet_tex_t *load_sheet_texture(const char *path) {
	sheet_tex_t *sheet_tex = (sheet_tex_t *)malloc(sizeof(sheet_tex_t));
	
	sheet_tex->texture = load_sdl_texture(path);

	return sheet_tex;
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
		tex_type = (texture_type_e *)malloc(sizeof(texture_type_e));
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
	TEXTURES = (texture_t **)malloc(sizeof(texture_t *) * NUM_TEXTURES);
	TEXTURE_MAP = hashmap_create(NUM_TEXTURES * 2, true, hash_string);

	// load textures
	json_object *texture_obj;
	const char *name, *tex_type_name, *rel_path;
	char file_path[80];
	bool transparent;
	size_t *texture_id;

	for (i = 0; i < NUM_TEXTURES; ++i) {
		TEXTURES[i] = (texture_t *)malloc(sizeof(texture_t));

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

		// load texture file
		switch (*tex_type) {
			case TEX_TEXTURE:
				TEXTURES[i]->tex.texture = load_sdl_texture(file_path);
				break;
			case TEX_SPRITE:
				TEXTURES[i]->tex.sprite = load_sprite(file_path);
				break;
			case TEX_VOXEL:
				TEXTURES[i]->tex.voxel = load_voxel_texture(file_path);
				break;
			case TEX_CONNECTED:
				printf("connected textures are not currently implemented.\n");
				exit(1);
				break;
			case TEX_SHEET:
				TEXTURES[i]->tex.sheet = load_sheet_texture(file_path);
				break;
		}

		// transparency
		if (*tex_type == TEX_VOXEL || *tex_type == TEX_SPRITE)
			transparent = false;
		else
			transparent = content_get_bool(texture_obj, "transparent");

		// sheet texture size
		if (*tex_type == TEX_SHEET) {
			v2i image_size, cell_size;

			if (content_has_key(texture_obj, "cell size"))
				cell_size = content_get_v2i(texture_obj, "cell size");
			else
				cell_size = VOXEL_DRAW_SIZE;

			SDL_QueryTexture(TEXTURES[i]->tex.sheet->texture, NULL, NULL, &image_size.x, &image_size.y);

			TEXTURES[i]->tex.sheet->size = v2i_div(image_size, cell_size);
		}

		// save to array and hashmap
		TEXTURES[i]->type = *tex_type;
		TEXTURES[i]->transparent = transparent;

		texture_id = (size_t *)malloc(sizeof(size_t));
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
				SDL_DestroyTexture(TEXTURES[i]->tex.sprite->texture);
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
				SDL_DestroyTexture(TEXTURES[i]->tex.sheet->texture);
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

block_tex_state_t block_tex_state_from(texture_type_e tex_type) {
	block_tex_state_t tex_state;

	tex_state.expose_mask = 0x7;

	switch (tex_type) {
		case TEX_VOXEL:
			tex_state.state.outline_mask = 0x0;
			break;
		case TEX_CONNECTED:
			tex_state.state.connected_mask = 0x0;
			break;
		case TEX_SHEET:
			tex_state.state.sheet_cell = (v2i){0, 0};
			break;
		default:
			break;
	}

	return tex_state;
}
