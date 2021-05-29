#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>
#include "vector.h"
#include "content.h"
#include "render.h"
#include "textures.h"
#include "render/textures.h"
#include "data_structures/hashmap.h"

const SDL_Rect VOXEL_TOP_RECT = {
	0,
	0,
	VOXEL_WIDTH,
	(VOXEL_WIDTH >> 1) - 1
};
const SDL_Rect VOXEL_SIDE_RECT = {
	0,
	(VOXEL_WIDTH >> 1) - 1,
	VOXEL_WIDTH >> 1,
	VOXEL_HEIGHT - (VOXEL_WIDTH >> 2)
};

texture_t **TEXTURES = NULL;
size_t NUM_TEXTURES;
hashmap_t *TEXTURE_MAP;

texture_t *DARK_VOXEL_TEXTURE;

SDL_Texture *load_sdl_texture(const char *path);
SDL_Texture *load_voxel_texture(const char *path);

texture_t *load_texture(const char *path, json_object *texture_obj,
						hashmap_t *tex_type_map, hashmap_t *tags_map) {
	texture_t *texture;
	array_t *tag_arr;
	const char *tag, *tex_type_name;
	size_t cur_tag_id = 0;
	size_t *tag_id;
	texture_type_e *tex_type;

	texture = malloc(sizeof(texture_t));

	// type
	tex_type_name = content_get_string(texture_obj, "type");
	tex_type = (texture_type_e *)hashmap_get(tex_type_map, tex_type_name);

	if (tex_type == NULL) {
		printf("\"%s\" is an unrecognized texture type.\n", tex_type_name);
		exit(1);
	}

	texture->type = *tex_type;

	// load texture
	if (texture->type == TEX_VOXEL)
		texture->texture = load_voxel_texture(path);
	else
		texture->texture = load_sdl_texture(path);

	// transparency
	if (*tex_type == TEX_VOXEL)
		texture->transparent = false;
	else
		texture->transparent = content_get_bool(texture_obj, "transparent");

	// connected tags
	if (content_has_key(texture_obj, "connected-tags")) {
		tag_arr = content_get_array(texture_obj, "connected-tags");

		texture->num_tags = tag_arr->size;
		texture->tags = malloc(sizeof(size_t) * texture->num_tags);

		for (size_t i = 0; i < texture->num_tags; ++i) {
			tag = json_object_get_string(tag_arr->items[i]);

			if (hashmap_get(tags_map, tag) == NULL) {
				tag_id = malloc(sizeof(size_t));
				*tag_id = cur_tag_id++;
				hashmap_set(tags_map, tag, tag_id);

				texture->tags[i] = *tag_id;
			} else {
				texture->tags[i] = *(size_t *)hashmap_get(tags_map, tag);
			}
		}

		array_destroy(tag_arr, false);
	} else {
		texture->num_tags = 0;
		texture->tags = NULL;
	}

	return texture;
}

void textures_load() {
	int i;

	// tex_type map
	const int num_tex_types = 4;
	char *tex_type_strings[] = {
		"texture",
		"voxel",
		"connected",
		"sheet",
	};
	texture_type_e *tex_type;
	hashmap_t *tex_type_map = hashmap_create(num_tex_types * 2, HASH_STRING);

	for (i = 0; i < num_tex_types; ++i) {
		tex_type = malloc(sizeof(texture_type_e));
		*tex_type = (texture_type_e)i;
		hashmap_set(tex_type_map, tex_type_strings[i], tex_type);
	}

	// tags
	hashmap_t *tags_map = hashmap_create(4, HASH_STRING);

	// json texture list
	json_object *file_obj;
	array_t *texture_objects;

	file_obj = content_load_file("assets/textures.json");
	texture_objects = content_get_array(file_obj, "textures");

	// set up globals
	NUM_TEXTURES = texture_objects->size;
	TEXTURES = malloc(sizeof(texture_t *) * NUM_TEXTURES);
	TEXTURE_MAP = hashmap_create(NUM_TEXTURES * 2, HASH_STRING);

	// load textures
	const char *name;
	char file_path[80];
	size_t *texture_id;

	for (i = 0; i < NUM_TEXTURES; ++i) {
		name = content_get_string(texture_objects->items[i], "name");
		sprintf(file_path, "assets/%s", content_get_string(texture_objects->items[i], "path"));

		TEXTURES[i] = load_texture(file_path, texture_objects->items[i], tex_type_map, tags_map);

		// save to array and hashmap
		texture_id = malloc(sizeof(size_t));
		*texture_id = i;
		hashmap_set(TEXTURE_MAP, name, texture_id);
	}

	// clean up and exit
	array_destroy(texture_objects, false);
	hashmap_destroy(tags_map, true);
	hashmap_destroy(tex_type_map, true);
	content_close_file(file_obj);

	DARK_VOXEL_TEXTURE = texture_from_key("dark");
}

void textures_destroy() {
	// SDL_Textures are freed with SDL_DestroyRenderer call
	for (size_t i = 0; i < NUM_TEXTURES; i++) {
		if (TEXTURES[i]->num_tags)
			free(TEXTURES[i]->tags);

		free(TEXTURES[i]);
	}

	DARK_VOXEL_TEXTURE = NULL;

	free(TEXTURES);
	TEXTURES = NULL;
	hashmap_destroy(TEXTURE_MAP, true);
	TEXTURE_MAP = NULL;
}

// only use when you actually need the surface data
SDL_Surface *load_sdl_surface(const char *path) {
	SDL_Surface *surface, *converted;

	if ((surface = IMG_Load(path)) == NULL) {
		printf("unable to load image %s:\n%s\n", path, IMG_GetError());
		exit(1);
	}

	converted = SDL_ConvertSurfaceFormat(surface, RENDER_FORMAT, 0);

	SDL_FreeSurface(surface);

	return converted;
}

SDL_Texture *load_sdl_texture(const char *path) {
	SDL_Texture *texture;
	SDL_Surface *surface;

	if ((surface = IMG_Load(path)) == NULL) {
		printf("unable to load image %s:\n%s\n", path, IMG_GetError());
		exit(1);
	}

	texture = SDL_CreateTextureFromSurface(renderer, surface);
	
	if (texture == NULL) {
		printf("unable to create texture from %s:\n%s\n", path, SDL_GetError());
		exit(1);
	}

	SDL_FreeSurface(surface);
	return texture;
}


// loads [path]_top.png and [path]_side.png
// this uses kinda complicated surface stuff in order to be able to store textures
// with the SDL_TEXTUREACCESS_STATIC access format
SDL_Texture *load_voxel_texture(const char *path) {
	SDL_Rect src_rect, dst_rect;
	SDL_Surface *surfaces[3];
	SDL_Surface *image;
	SDL_Texture *texture;

	image = load_sdl_surface(path);

	surfaces[2] = SDL_CreateRGBSurfaceWithFormat(0, VOXEL_TOP_RECT.w, VOXEL_TOP_RECT.h, 32, RENDER_FORMAT);
	surfaces[1] = SDL_CreateRGBSurfaceWithFormat(0, VOXEL_SIDE_RECT.w, VOXEL_SIDE_RECT.h, 32, RENDER_FORMAT);
	surfaces[0] = SDL_CreateRGBSurfaceWithFormat(0, surfaces[1]->w, surfaces[1]->h, 32, RENDER_FORMAT);

	SDL_BlitSurface(image, &VOXEL_TOP_RECT, surfaces[2], NULL);
	SDL_BlitSurface(image, &VOXEL_SIDE_RECT, surfaces[1], NULL);

	// flip surfaces (yeah this is hacky and probably slow, but not performance-critical whatsoever)
	src_rect = (SDL_Rect){0, 0, 1, surfaces[1]->h};
	dst_rect = src_rect;

	for (src_rect.x = 0; src_rect.x < surfaces[1]->w; ++src_rect.x) {
		dst_rect.x = surfaces[1]->w - src_rect.x - 1;
		SDL_BlitSurface(surfaces[1], &src_rect, surfaces[0], &dst_rect);
	}

	texture = render_cached_voxel_texture(surfaces);

	for (int i = 0; i < 3; ++i)
		SDL_FreeSurface(surfaces[i]);

	SDL_FreeSurface(image);

	return texture;
}

texture_t *texture_from_key(const char *key) {
	size_t *value = hashmap_get(TEXTURE_MAP, key);

	if (value == NULL) {
		printf("key not found in TEXTURE_MAP: %s\n", key);
		exit(1);
	}

	return TEXTURES[*value];
}

texture_state_t texture_state_from_type(texture_type_e tex_type) {
	texture_state_t tex_state;

	switch (tex_type) {
	case TEX_TEXTURE: // for non-transparent textures
	case TEX_VOXEL:
		tex_state.outline_mask = 0x0;
		break;
	case TEX_CONNECTED:
		tex_state.connected_mask = 0x0;
		break;
	case TEX_SHEET:
			tex_state.cell = (v2i){0, 0};
			break;
	}

	return tex_state;
}
