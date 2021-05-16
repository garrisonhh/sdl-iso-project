#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>
#include "vector.h"
#include "content.h"
#include "render.h"
#include "textures.h"
#include "render_textures.h"
#include "data_structures/hashmap.h"
#include "data_structures/hash_functions.h"

texture_t **TEXTURES = NULL;
size_t NUM_TEXTURES;
hashmap_t *TEXTURE_MAP;

voxel_tex_t *DARK_VOXEL_TEXTURE;

SDL_Texture *load_sdl_texture(const char *path);
sprite_t *load_sprite(const char *path, json_object *obj, hashmap_t *sprite_type_map);
voxel_tex_t* load_voxel_texture(const char *path);
connected_tex_t *load_connected_texture(const char *path);
sheet_tex_t *load_sheet_texture(const char *path, json_object *obj);

void textures_load() {
	int i, j;

	// tex_type map
	const int num_tex_types = 5;
	char *tex_type_strings[] = {
		"texture",
		"sprite",
		"voxel",
		"connected",
		"sheet",
	};
	texture_type_e *tex_type;
	hashmap_t *tex_type_map = hashmap_create(num_tex_types * 2, false, hash_string);

	for (i = 0; i < num_tex_types; ++i) {
		tex_type = malloc(sizeof(texture_type_e));
		*tex_type = (texture_type_e)i;
		hashmap_set(tex_type_map, tex_type_strings[i], strlen(tex_type_strings[i]), tex_type);
	}

	// sprite_type map
	const int num_sprite_types = 4;
	char *sprite_type_strings[] = {
		"static",
		"human-body",
		"human-back-hands",
		"human-front-hands",
	};
	sprite_type_e *sprite_type;
	hashmap_t *sprite_type_map = hashmap_create(num_sprite_types * 2, false, hash_string);

	for (i = 0; i < num_sprite_types; ++i) {
		sprite_type = malloc(sizeof(sprite_type_e));
		*sprite_type = (sprite_type_e)i;
		hashmap_set(sprite_type_map, sprite_type_strings[i], strlen(sprite_type_strings[i]), sprite_type);
	}

	// tags
	hashmap_t *tags_map = hashmap_create(4, true, hash_string);
	array_t *tag_arr;
	const char *tag;
	int len_tag;
	size_t cur_tag_id = 0;
	size_t *tag_id;

	// json texture list
	json_object *file_obj;
	array_t *texture_objects;

	file_obj = content_load_file("assets/textures.json");
	texture_objects = content_array_from_obj(file_obj);

	// set up globals
	NUM_TEXTURES = texture_objects->size;
	TEXTURES = malloc(sizeof(texture_t *) * NUM_TEXTURES);
	TEXTURE_MAP = hashmap_create(NUM_TEXTURES * 2, true, hash_string);

	// load textures
	json_object *texture_obj;
	const char *name, *tex_type_name, *rel_path;
	char file_path[80];
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

		TEXTURES[i]->type = *tex_type;

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
				TEXTURES[i]->tex.sprite = load_sprite(file_path, texture_obj, sprite_type_map);
				break;
			case TEX_VOXEL:
				TEXTURES[i]->tex.voxel = load_voxel_texture(file_path);
				break;
			case TEX_CONNECTED:
				TEXTURES[i]->tex.connected = load_connected_texture(file_path);
				break;
			case TEX_SHEET:
				TEXTURES[i]->tex.sheet = load_sheet_texture(file_path, texture_obj);
				break;
		}

		// transparency
		if (*tex_type == TEX_VOXEL || *tex_type == TEX_SPRITE)
			TEXTURES[i]->transparent = false;
		else
			TEXTURES[i]->transparent = content_get_bool(texture_obj, "transparent");

		// connected tags
		if (content_has_key(texture_obj, "connected-tags")) {
			tag_arr = content_get_array(texture_obj, "connected-tags");

			TEXTURES[i]->num_tags = tag_arr->size;
			TEXTURES[i]->tags = malloc(sizeof(size_t) * TEXTURES[i]->num_tags);

			for (j = 0; j < TEXTURES[i]->num_tags; ++j) {
				tag = json_object_get_string(tag_arr->items[j]);
				len_tag = strlen(tag);

				if (hashmap_contains(tags_map, (char *)tag, len_tag)) {
					TEXTURES[i]->tags[j] = *(size_t *)hashmap_get(tags_map, (char *)tag, len_tag);
				} else {
					tag_id = malloc(sizeof(size_t));
					*tag_id = cur_tag_id++;
					hashmap_set(tags_map, (char *)tag, len_tag, tag_id);

					TEXTURES[i]->tags[j] = *tag_id;
				}
			}
		} else {
			TEXTURES[i]->num_tags = 0;
			TEXTURES[i]->tags = NULL;
		}

		// save to array and hashmap
		texture_id = malloc(sizeof(size_t));
		*texture_id = i;
		hashmap_set(TEXTURE_MAP, (char *)name, strlen(name), texture_id);
	}

	// clean up and exit
	array_destroy(texture_objects, false);
	hashmap_destroy(tags_map, true);
	hashmap_destroy(tex_type_map, true);
	hashmap_destroy(sprite_type_map, true);
	content_close_file(file_obj);

	DARK_VOXEL_TEXTURE = texture_from_key("dark")->tex.voxel;
}

void textures_destroy() {
	int j;

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
				for (j = 0; j < 7; ++j)
					SDL_DestroyTexture(TEXTURES[i]->tex.voxel->textures[j]);
				free(TEXTURES[i]->tex.voxel);
				break;
			case TEX_CONNECTED:
				for (j = 0; j < 6; ++j)
					SDL_DestroyTexture(TEXTURES[i]->tex.connected->directions[j]);
				SDL_DestroyTexture(TEXTURES[i]->tex.connected->center);
				free(TEXTURES[i]->tex.connected);
				break;
			case TEX_SHEET:
				SDL_DestroyTexture(TEXTURES[i]->tex.sheet->sheet);
				free(TEXTURES[i]->tex.sheet);
				break;
		}
		
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

sprite_t *load_sprite(const char *path, json_object *obj, hashmap_t *sprite_type_map) {
	sprite_t *sprite = malloc(sizeof(sprite_t));

	sprite->sheet = load_sdl_texture(path);

	if (content_has_key(obj, "sprite-type")) {
		const char *sprite_type_name;
		sprite_type_e *sprite_type;

		sprite_type_name = content_get_string(obj, "sprite-type");
		sprite_type = hashmap_get(sprite_type_map, (char *)sprite_type_name, strlen(sprite_type_name));

		if (sprite_type == NULL) {
			printf("\"%s\" is not a valid sprite type.\n", sprite_type_name);
			exit(1);
		}

		sprite->type = *sprite_type;
	} else {
		sprite->type = SPRITE_STATIC;
	}

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
		sprite->anim_lengths = malloc(sizeof(int));
		sprite->num_anims = 1;
		sprite->anim_lengths[0] = 1;
	}

	return sprite;
}

// loads [path]_top.png and [path]_side.png
// this uses kinda complicated surface stuff in order to be able to store textures
// with the SDL_TEXTUREACCESS_STATIC access format
voxel_tex_t* load_voxel_texture(const char *path) {
	size_t len_path;
	char top_path[100], side_path[100];
	SDL_Rect src_rect, dst_rect;
	SDL_Surface *surfaces[3];
	voxel_tex_t *voxel_tex;

	voxel_tex = malloc(sizeof(voxel_tex_t));
	len_path = strlen(path) - 4;

	strncpy(top_path, path, 99);
	strncpy(side_path, path, 99);
	top_path[len_path] = 0;
	side_path[len_path] = 0;
	strcat(top_path, "_top.png");
	strcat(side_path, "_side.png");

	surfaces[2] = load_sdl_surface(top_path);
	surfaces[1] = load_sdl_surface(side_path);
	surfaces[0] = SDL_CreateRGBSurfaceWithFormat(0, surfaces[1]->w, surfaces[1]->h, 32, RENDER_FORMAT);

	// flip surfaces (yeah this is hacky and probably slow, but not performance-critical whatsoever)
	src_rect = (SDL_Rect){0, 0, 1, surfaces[1]->h};
	dst_rect = src_rect;

	for (src_rect.x = 0; src_rect.x < surfaces[1]->w; ++src_rect.x) {
		dst_rect.x = surfaces[1]->w - src_rect.x - 1;
		SDL_BlitSurface(surfaces[1], &src_rect, surfaces[0], &dst_rect);
	}

	for (unsigned i = 1; i < 8; ++i)
		voxel_tex->textures[i - 1] = render_cached_voxel_texture(surfaces, i);

	for (int i = 0; i < 3; ++i)
		SDL_FreeSurface(surfaces[i]);

	return voxel_tex;
}

connected_tex_t *load_connected_texture(const char *path) {
	int i;

	// textures
	connected_tex_t *connected_tex = malloc(sizeof(connected_tex_t));

	SDL_Surface *sheet = load_sdl_surface(path);
	SDL_Surface *surface;
	SDL_Texture *texture;
	SDL_Rect src_rect = {0, 0, VOXEL_WIDTH, VOXEL_HEIGHT};

	for (i = 0; i < 7; ++i) {
		surface = SDL_CreateRGBSurfaceWithFormat(0, VOXEL_WIDTH, VOXEL_HEIGHT, 32, RENDER_FORMAT);

		SDL_BlitSurface(sheet, &src_rect, surface, NULL);

		texture = SDL_CreateTextureFromSurface(renderer, surface);

		if (i == 6)
			connected_tex->center = texture;
		else
			connected_tex->directions[i] = texture;

		SDL_FreeSurface(surface);
		src_rect.x += VOXEL_WIDTH;
	}

	SDL_FreeSurface(sheet);

	return connected_tex;
}

sheet_tex_t *load_sheet_texture(const char *path, json_object *obj) {
	sheet_tex_t *sheet_tex = malloc(sizeof(sheet_tex_t));
	v2i image_size, cell_size;
	
	sheet_tex->sheet = load_sdl_texture(path);

	if (content_has_key(obj, "cell-size"))
		cell_size = content_get_v2i(obj, "cell-size");
	else
		cell_size = (v2i){VOXEL_WIDTH, VOXEL_HEIGHT};

	SDL_QueryTexture(sheet_tex->sheet, NULL, NULL, &image_size.x, &image_size.y);

	sheet_tex->sheet_size = v2i_div(image_size, cell_size);

	return sheet_tex;
}

texture_t *texture_from_key(const char *key) {
	size_t *value = hashmap_get(TEXTURE_MAP, (char *)key, strlen(key));

	if (value == NULL) {
		printf("key not found in TEXTURE_MAP: %s\n", key);
		exit(1);
	}
	
	return TEXTURES[*value];
}

sprite_t *sprite_from_key(const char *key) {
	return texture_from_key(key)->tex.sprite;
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
