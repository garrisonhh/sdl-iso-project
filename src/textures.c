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

texture_t **textures = NULL;
size_t num_textures;
hashmap_t *texture_table;

voxel_tex_t *VOID_VOXEL_TEXTURE;

SDL_Texture *load_sdl_texture(char *path) {
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

sprite_t *load_sprite(char *path) {
	sprite_t *sprite = (sprite_t *)malloc(sizeof(sprite_t));

	sprite->texture = load_sdl_texture(path);
	SDL_QueryTexture(sprite->texture, NULL, NULL, &sprite->size.x, &sprite->size.y);
	sprite->pos = (v2i){-(sprite->size.x >> 1), -sprite->size.y};

	return sprite;
}

// loads [path]_top.png and [path]_side.png
voxel_tex_t* load_voxel_texture(char *path) {
	char top_path[100], side_path[100];

	strcpy(top_path, path);
	strcpy(side_path, path);
	strcat(top_path, "_top.png");
	strcat(side_path, "_side.png");

	voxel_tex_t* voxel_tex = (voxel_tex_t *)malloc(sizeof(voxel_tex_t));
	voxel_tex->top = load_sdl_texture(top_path);
	voxel_tex->side = load_sdl_texture(side_path);

	return voxel_tex;
}

connected_tex_t *load_connected_texture(char *path) {
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

// TODO use new json interface
void textures_load(json_object *file_obj) {
	const char *name;
	char path[100];
	char *rel_path, *obj_str;
	size_t *arr_index;
	size_t i, num_tex_types;
	json_object *tex_arr_obj, *current_tex, *obj;

	hashmap_t *tex_type_table;
	texture_type_e *tex_type_ptr;

	num_tex_types = 5;
	char *tex_type_strings[] = {
		"texture",
		"sprite",
		"voxel",
		"connected",
		"sheet",
	};

	tex_type_table = hashmap_create(num_tex_types * 1.3 + 1, true, hash_string);
	tex_arr_obj = json_object_object_get(file_obj, "textures");
	num_textures = json_object_array_length(tex_arr_obj);
	textures = (texture_t **)calloc(num_textures, sizeof(texture_t *));
	texture_table = hashmap_create(num_textures * 1.3 + 1, true, hash_string);

	for (i = 0; i < num_tex_types; i++) {
		tex_type_ptr = (texture_type_e *)malloc(sizeof(texture_type_e));
		*tex_type_ptr = (texture_type_e)i;
		hashmap_set(tex_type_table, tex_type_strings[i], strlen(tex_type_strings[i]), tex_type_ptr);
	}

	for (i = 0; i < num_textures; i++) {
		textures[i] = (texture_t *)malloc(sizeof(texture_t));

		current_tex = json_object_array_get_idx(tex_arr_obj, i);

		// name and custom path (if defined)
		obj = json_object_object_get(current_tex, "name");
		name = json_object_get_string(obj);

		if ((obj = json_object_object_get(current_tex, "path")) != NULL)
			rel_path = (char *)json_object_get_string(obj);
		else
			rel_path = (char *)name;

		if (strlen(rel_path) > 80) { // no hacking!
			printf("relative texture path \"%s\" is too long.\n", rel_path);
			exit(1);
		}

		sprintf(path, "assets/%s", rel_path); 

		// type
		obj = json_object_object_get(current_tex, "type");
		obj_str = (char *)json_object_get_string(obj);
		tex_type_ptr = hashmap_get(tex_type_table, obj_str, strlen(obj_str));

		if (tex_type_ptr == NULL) {
			printf("unknown texture type for texture \"%s\".\n", name);
			exit(1);
		}

		textures[i]->type = *tex_type_ptr;

		// transparency
		if (textures[i]->type == TEX_VOXEL) {
			textures[i]->transparent = false;
		} else {
			obj = json_object_object_get(current_tex, "transparent");
			textures[i]->transparent = json_object_get_boolean(obj);
		}

		// load image
		switch (textures[i]->type) {
			case TEX_TEXTURE:
				strcat(path, ".png");
				textures[i]->tex.texture = load_sdl_texture(path);
				break;	
			case TEX_SPRITE:
				strcat(path, ".png");
				textures[i]->tex.sprite = load_sprite(path);
				break;
			case TEX_VOXEL:
				textures[i]->tex.voxel = load_voxel_texture(path);
				break;
			case TEX_CONNECTED:
				textures[i]->tex.connected = load_connected_texture(path);
				break;
			case TEX_SHEET:
				strcat(path, ".png");
				textures[i]->tex.sheet = (sheet_tex_t *)malloc(sizeof(sheet_tex_t));
				textures[i]->tex.sheet->texture = load_sdl_texture(path);
				break;
		}

		// sheet texture cell and sheet sizes
		if (textures[i]->type == TEX_SHEET) {
			v2i sheet_size;

			if ((obj = json_object_object_get(current_tex, "cell size")) == NULL) {
				printf("texture sheet does not provide \"cell size\".\n");
				exit(1);
			}

			if (json_object_array_length(obj) != 2) {
				printf("cell size for sheet \"%s\" is not in format [w, h].\n", name);
				exit(1);
			}

			SDL_QueryTexture(textures[i]->tex.sheet->texture, NULL, NULL, &sheet_size.x, &sheet_size.y);
			textures[i]->tex.sheet->size.x = sheet_size.x / VOXEL_WIDTH;
			textures[i]->tex.sheet->size.y = sheet_size.y / VOXEL_HEIGHT;
		}

		// add to indexing hash table
		arr_index = (size_t *)malloc(sizeof(size_t));
		*arr_index = i;
		hashmap_set(texture_table, (char *)name, strlen(name), arr_index);
	}

	hashmap_destroy(tex_type_table, true);

	VOID_VOXEL_TEXTURE = texture_ptr_from_key("void")->tex.voxel;
}

void textures_destroy() {
	for (size_t i = 0; i < num_textures; i++) {
		switch (textures[i]->type) {
			case TEX_TEXTURE:
				SDL_DestroyTexture(textures[i]->tex.texture);
				break;
			case TEX_SPRITE:
				SDL_DestroyTexture(textures[i]->tex.sprite->texture);
				free(textures[i]->tex.sprite);
				break;
			case TEX_VOXEL:
				SDL_DestroyTexture(textures[i]->tex.voxel->top);
				SDL_DestroyTexture(textures[i]->tex.voxel->side);
				free(textures[i]->tex.voxel);
				break;
			case TEX_CONNECTED:
				SDL_DestroyTexture(textures[i]->tex.connected->base);
				SDL_DestroyTexture(textures[i]->tex.connected->top);
				SDL_DestroyTexture(textures[i]->tex.connected->bottom);
				SDL_DestroyTexture(textures[i]->tex.connected->front);
				SDL_DestroyTexture(textures[i]->tex.connected->back);
				free(textures[i]->tex.connected);
				break;
			case TEX_SHEET:
				SDL_DestroyTexture(textures[i]->tex.sheet->texture);
				free(textures[i]->tex.sheet);
				break;
		}

		free(textures[i]);
	}

	VOID_VOXEL_TEXTURE = NULL;

	free(textures);
	textures = NULL;
	hashmap_destroy(texture_table, true);
	texture_table = NULL;
}

texture_t *texture_ptr_from_key(char *key) {
	size_t *value;

	if ((value = (size_t *)hashmap_get(texture_table, key, strlen(key))) == NULL) {
		printf("key not found in texture_table: %s\n", key);
		exit(1);
	}
	
	return textures[*value];
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
