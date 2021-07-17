#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <json-c/json.h>
#include <ghh/hashmap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "textures.h"
#include "content.h"
#include "render.h"
#include "meta.h"
#include "render/textures.h"
#include "lib/vector.h"

struct pre_texture_t {
	SDL_Surface *surface;
	SDL_Rect rect;
};
typedef struct pre_texture_t pre_texture_t;

struct tex_load_context_t {
	array_t *pre_textures;
	v2i pos; // location for next surface

	hashmap_t *tex_type_map, *tags_map;
};
typedef struct tex_load_context_t tex_load_context_t;

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

SDL_Texture *TEXTURE_ATLAS = NULL;
texture_t **TEXTURES = NULL;
size_t NUM_TEXTURES;
hashmap_t *TEXTURE_MAP;

const SDL_Rect TEXTURE_OUTLINES_RECT = {0, 0, VOXEL_WIDTH + 2, VOXEL_HEIGHT + 1};
texture_t *DARK_VOXEL_TEXTURE = NULL;

SDL_Surface *load_sdl_surface(const char *path);
SDL_Surface *load_voxel_surface(const char *path);
SDL_Surface *load_connected_surface(const char *path);

texture_t *load_texture(tex_load_context_t *context, json_object *texture_obj) {
	const char *tag, *tex_type_name;
	char path[PATH_LEN];
	size_t cur_tag_id = 0;
	size_t *tag_id;
	texture_t *texture;
	array_t *tag_arr;
	texture_type_e *tex_type;
	pre_texture_t *pre_texture;

	texture = malloc(sizeof(texture_t));

	// type
	tex_type_name = content_get_string(texture_obj, "type");
	tex_type = hashmap_get(context->tex_type_map, tex_type_name);

	if (tex_type == NULL) {
		printf("\"%s\" is an unrecognized texture type.\n", tex_type_name);
		exit(1);
	}

	texture->type = *tex_type;

	// load pre_texture for atlas, store atlas rect
	strcpy(path, content_get_string(texture_obj, "path"));

	pre_texture = malloc(sizeof(pre_texture_t));

	switch (texture->type) {
	case TEX_VOXEL:
		pre_texture->surface = load_voxel_surface(path);
		break;
	case TEX_CONNECTED:
		pre_texture->surface = load_connected_surface(path);
		break;
	default:
		pre_texture->surface = load_sdl_surface(path);
		break;
	}

	pre_texture->rect = (SDL_Rect){
		context->pos.x,
		context->pos.y,
		pre_texture->surface->w,
		pre_texture->surface->h
	};

	texture->atlas_rect = (SDL_Rect){
		context->pos.x,
		context->pos.y,
		VOXEL_WIDTH,
		VOXEL_HEIGHT
	};

	context->pos.y += pre_texture->rect.h;
	array_push(context->pre_textures, pre_texture);

	// transparency
	if (texture->type == TEX_VOXEL)
		texture->transparent = false;
	else
		texture->transparent = content_get_bool(texture_obj, "transparent");

	// connected tags
	if (content_has_key(texture_obj, "connected-tags")) {
		tag_arr = content_get_array(texture_obj, "connected-tags");

		texture->num_tags = array_size(tag_arr);
		texture->tags = malloc(sizeof(size_t) * texture->num_tags);

		for (size_t i = 0; i < texture->num_tags; ++i) {
			tag = json_object_get_string(array_get(tag_arr, i));

			if (hashmap_get(context->tags_map, tag) == NULL) {
				tag_id = malloc(sizeof(size_t));
				*tag_id = cur_tag_id++;
				hashmap_set(context->tags_map, tag, tag_id);

				texture->tags[i] = *tag_id;
			} else {
				texture->tags[i] = *(size_t *)hashmap_get(context->tags_map, tag);
			}
		}

		array_destroy(tag_arr, false);
	} else {
		texture->num_tags = 0;
		texture->tags = NULL;
	}

	return texture;
}

void textures_context_populate(tex_load_context_t *context) {
	// atlas build vars
	context->pre_textures = array_create(0);
	context->pos = (v2i){0, 0};

	// tex_type and tags maps
	const char *tex_type_strings[NUM_TEXTURE_TYPES] = {
		"texture",
		"voxel",
		"connected",
		"sheet"
	};
	texture_type_e *tex_type;

	context->tex_type_map = hashmap_create(NUM_TEXTURE_TYPES * 2, -1, false);

	for (int i = 0; i < NUM_TEXTURE_TYPES; ++i) {
		tex_type = malloc(sizeof(texture_type_e));
		*tex_type = (texture_type_e)i;
		hashmap_set(context->tex_type_map, tex_type_strings[i], tex_type);
	}

	context->tags_map = hashmap_create(4, -1, true);
}

void textures_build_atlas(tex_load_context_t *context) {
	size_t i;
	int w = 0, h = 0;
	SDL_Surface *atlas_surf;
	pre_texture_t *pre_texture;

	// find atlas dimensions by adding all rect heights and finding max rect width
	for (i = 0; i < array_size(context->pre_textures); ++i) {
		pre_texture = array_get(context->pre_textures, i);

		h += pre_texture->rect.h;

		if (pre_texture->rect.w > w)
			w = pre_texture->rect.w;
	}

	// build atlas
	atlas_surf = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, RENDER_FORMAT);

	for (i = 0; i < array_size(context->pre_textures); ++i) {
		pre_texture = array_get(context->pre_textures, i);

		if (SDL_BlitSurface(pre_texture->surface, NULL, atlas_surf, &pre_texture->rect) < 0) {
			printf("blit to atlas surf failed:\n%s\n", SDL_GetError());
			exit(1);
		}

		SDL_FreeSurface(pre_texture->surface);
	}

	TEXTURE_ATLAS = SDL_CreateTextureFromSurface(RENDERER, atlas_surf);

	if (TEXTURE_ATLAS == NULL) {
		printf("atlas texture creation failed:\n%s\n", SDL_GetError());
		exit(1);
	} else {
		printf("successfully generated atlas with dimensions %i %i\n", atlas_surf->w, atlas_surf->h);
	}

	SDL_FreeSurface(atlas_surf);
}

void textures_load_outlines(tex_load_context_t *context) {
	pre_texture_t *pre_texture = malloc(sizeof(pre_texture_t));
	SDL_Surface *outlines = load_sdl_surface("blocks/outlines.png");

	pre_texture->surface = render_voxel_outline_surface(outlines);
	pre_texture->rect = (SDL_Rect){0, 0, pre_texture->surface->w, pre_texture->surface->h};

	array_push(context->pre_textures, pre_texture);
	context->pos.y += pre_texture->rect.h;

	SDL_FreeSurface(outlines);
}

void textures_load() {
	tex_load_context_t context;
	textures_context_populate(&context);

	// json texture list
	json_object *file_obj;
	array_t *texture_objects;

	file_obj = content_load_file("textures.json");
	texture_objects = content_get_array(file_obj, "textures");

	// set up globals
	NUM_TEXTURES = array_size(texture_objects);
	TEXTURES = malloc(sizeof(texture_t *) * NUM_TEXTURES);
	TEXTURE_MAP = hashmap_create(NUM_TEXTURES * 2, -1, true);

	// load textures
	const char *name;
	size_t *texture_id;

	textures_load_outlines(&context);

	for (int i = 0; i < NUM_TEXTURES; ++i) {
		name = content_get_string(array_get(texture_objects, i), "name");

		TEXTURES[i] = load_texture(&context, array_get(texture_objects, i));

		// save to array and hashmap
		texture_id = malloc(sizeof(size_t));
		*texture_id = i;
		hashmap_set(TEXTURE_MAP, name, texture_id);
	}

	textures_build_atlas(&context);

	// clean up
	array_destroy(texture_objects, false);
	array_destroy(context.pre_textures, true);
	hashmap_destroy(context.tags_map, true);
	hashmap_destroy(context.tex_type_map, true);
	content_close_file(file_obj);

	// special
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

SDL_Surface *load_sdl_surface(const char *asset_path) {
	SDL_Surface *surface, *converted;
	char path[PATH_LEN];

	content_asset_path(path, asset_path);

	if ((surface = IMG_Load(path)) == NULL) {
		printf("unable to load image %s:\n%s\n", path, IMG_GetError());
		exit(1);
	}

	converted = SDL_ConvertSurfaceFormat(surface, RENDER_FORMAT, 0);

	SDL_FreeSurface(surface);

	return converted;
}

// currently only used in sprites_load
SDL_Texture *load_sdl_texture(const char *asset_path) {
	SDL_Texture *texture;
	SDL_Surface *surface;
	char path[PATH_LEN];

	content_asset_path(path, asset_path);

	if ((surface = IMG_Load(path)) == NULL) {
		printf("unable to load image %s:\n%s\n", path, IMG_GetError());
		exit(1);
	}

	texture = SDL_CreateTextureFromSurface(RENDERER, surface);

	if (texture == NULL) {
		printf("unable to create texture from %s:\n%s\n", path, SDL_GetError());
		exit(1);
	}

	SDL_FreeSurface(surface);
	return texture;
}

// loads [path]_top.png and [path]_side.png
SDL_Surface *load_voxel_surface(const char *path) {
	SDL_Rect src_rect, dst_rect;
	SDL_Surface *surfaces[3];
	SDL_Surface *image, *voxel_surface;

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

	voxel_surface = render_cached_voxel_surface(surfaces);

	for (int i = 0; i < 3; ++i)
		SDL_FreeSurface(surfaces[i]);

	SDL_FreeSurface(image);

	return voxel_surface;
}

SDL_Surface *load_connected_surface(const char *path) {
	SDL_Surface *sheet = load_sdl_surface(path);
	SDL_Surface *expanded = render_cached_connected_surface(sheet);

	SDL_FreeSurface(sheet);

	return expanded;
}

texture_t *texture_from_key(const char *key) {
	size_t *value = hashmap_get(TEXTURE_MAP, key);

	if (value == NULL) {
		printf("key not found in TEXTURE_MAP: %s\n", key);
		exit(1);
	}

	return TEXTURES[*value];
}
