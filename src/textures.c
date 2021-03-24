#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <json-c/json.h>
#include <stdlib.h>
#include "vector.h"
#include "media.h"
#include "render.h"
#include "textures.h"
#include "hash.h"

texture_t **textures = NULL;
size_t num_textures;
hash_table *texture_table;

voxel_tex_t *VOID_VOXEL_TEXTURE;

SDL_Rect sdl_tex_rect;
SDL_Rect voxel_tex_rects[3];
Uint8 voxel_tex_shades[] = { // flat shading values (out of 255) for each side t-l-r
	255,
	223,
	191
};

// corresponds to texture_type enum values; used for json parsing

// workaround for C's weird global constant rules
void textures_init() {
	sdl_tex_rect = (SDL_Rect){
		-(VOXEL_WIDTH >> 1),
		-VOXEL_Z_HEIGHT,
		VOXEL_WIDTH,
		VOXEL_HEIGHT
	};

	SDL_Rect voxel_tex_rects_tmp[3] = {
		{
			-(VOXEL_WIDTH >> 1),
			-VOXEL_Z_HEIGHT,
			VOXEL_WIDTH,
			VOXEL_WIDTH >> 1
		},
		{
			-(VOXEL_WIDTH >> 1),
			-VOXEL_Z_HEIGHT + (VOXEL_WIDTH >> 2),
			VOXEL_WIDTH >> 1,
			VOXEL_HEIGHT - (VOXEL_WIDTH >> 2)
		},
		{
			0,
			-VOXEL_Z_HEIGHT + (VOXEL_WIDTH >> 2),
			VOXEL_WIDTH >> 1,
			VOXEL_HEIGHT - (VOXEL_WIDTH >> 2)
		},
	};

	memcpy(voxel_tex_rects, voxel_tex_rects_tmp, sizeof voxel_tex_rects_tmp);
}

void textures_load(json_object *file_obj) {
	size_t i;
	const char *name;
	char path[100];
	char *rel_path;
	size_t *arr_index;
	json_object *tex_arr_obj, *current_tex, *obj;
	hash_table *tex_type_table;
	size_t num_tex_types = 3;
	texture_type *tex_type_ptr;

	tex_type_table = hash_table_create(num_tex_types * 1.3 + 1);
	tex_arr_obj = json_object_object_get(file_obj, "textures");
	num_textures = json_object_array_length(tex_arr_obj);
	textures = (texture_t **)calloc(num_textures, sizeof(texture_t *));
	texture_table = hash_table_create(num_textures * 1.3 + 1);

	// type hash table
	char *tex_type_strings[] = {
		"texture",
		"voxel",
		"connected"
	};

	for (i = 0; i < num_tex_types; i++) {
		tex_type_ptr = (texture_type *)malloc(sizeof(texture_type *));
		*tex_type_ptr = (texture_type)i;
		hash_set(tex_type_table, tex_type_strings[i], tex_type_ptr);
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
		tex_type_ptr = hash_get(tex_type_table, (char *)json_object_get_string(obj));

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
			case TEX_TEXTURE:;
				strcat(path, ".png");
				textures[i]->texture = load_sdl_texture(path);
				break;	
			case TEX_VOXEL:
				textures[i]->voxel_tex = load_voxel_texture(path);
				break;
			case TEX_CONNECTED:
				textures[i]->connected_tex = load_connected_texture(path);
				break;
		}

		// add to indexing hash table
		arr_index = (size_t *)malloc(sizeof(size_t));
		*arr_index = i;
		hash_set(texture_table, (char *)name, arr_index);
	}

	hash_table_deep_destroy(tex_type_table);

	VOID_VOXEL_TEXTURE = textures[texture_index("void")]->voxel_tex;
}

void textures_destroy() {
	for (size_t i = 0; i < num_textures; i++) {
		switch (textures[i]->type) {
			case TEX_TEXTURE:
				SDL_DestroyTexture(textures[i]->texture);
				break;
			case TEX_VOXEL:
				SDL_DestroyTexture(textures[i]->voxel_tex->top);
				SDL_DestroyTexture(textures[i]->voxel_tex->side);
				free(textures[i]->voxel_tex);
				break;
			case TEX_CONNECTED:
				SDL_DestroyTexture(textures[i]->connected_tex->base);
				SDL_DestroyTexture(textures[i]->connected_tex->top);
				SDL_DestroyTexture(textures[i]->connected_tex->bottom);
				SDL_DestroyTexture(textures[i]->connected_tex->front);
				SDL_DestroyTexture(textures[i]->connected_tex->back);
				free(textures[i]->connected_tex);
				break;
		}

		free(textures[i]);
	}

	VOID_VOXEL_TEXTURE = NULL;

	free(textures);
	textures = NULL;
	hash_table_deep_destroy(texture_table);
	texture_table = NULL;
}

size_t texture_index(char *key) {
	size_t *value;

	if ((value = (size_t *)hash_get(texture_table, key)) != NULL)
		return *value;

	printf("key not found in texture_table: %s\n", key);
	exit(1);
}

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

void render_sdl_texture(SDL_Texture *texture, v2i pos) {
	SDL_Rect draw_rect = sdl_tex_rect;
	draw_rect.x += pos.x;
	draw_rect.y += pos.y;

	SDL_RenderCopy(renderer, texture, NULL, &draw_rect);
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

// masks use only the last 3 bits; right-left-top order (corresponding to XYZ)
// void_mask determines sides which will be displayed as void (fully black)
void render_voxel_texture(voxel_tex_t *voxel_texture, v2i pos, uint8_t expose_mask, uint8_t void_mask) {
	uint8_t shade;
	bool exposed, voided;
	SDL_Rect draw_rect;
	voxel_tex_t *cur_texture;

	for (int i = 2; i >= 0; i--) {
		if ((exposed = (expose_mask >> i) & 0x1)
		 || (voided = (void_mask >> i) & 0x1)) {
			draw_rect = voxel_tex_rects[i];
			draw_rect.x += pos.x;
			draw_rect.y += pos.y;

			cur_texture = (void_mask >> i) & 1 ? VOID_VOXEL_TEXTURE : voxel_texture;

			if (exposed) {
				shade = voxel_tex_shades[i];

				if (i == 0)
					SDL_SetTextureColorMod(cur_texture->top, shade, shade, shade);
				else
					SDL_SetTextureColorMod(cur_texture->side, shade, shade, shade);
			}

			switch (i) {
				case 0:
					SDL_RenderCopy(renderer, cur_texture->top, NULL, &draw_rect);
					break;
				case 1:
					SDL_RenderCopy(renderer, cur_texture->side, NULL, &draw_rect);
					break;
				case 2:
					SDL_RenderCopyEx(renderer, cur_texture->side, NULL, &draw_rect,
							         0, NULL, SDL_FLIP_HORIZONTAL);
					break;
			}
		}
	}
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

void render_connected_texture(connected_tex_t *connected_tex, v2i pos, uint8_t connected_mask) {
	SDL_Rect draw_rect = sdl_tex_rect;
	SDL_Texture *textures[6] = {
		connected_tex->bottom, connected_tex->top,
		connected_tex->back, connected_tex->front,
		connected_tex->back, connected_tex->front,
	};
	int i, j;

	draw_rect.x += pos.x;
	draw_rect.y += pos.y;

	SDL_RenderCopy(renderer, connected_tex->base, NULL, &draw_rect);

	for (i = 0; i <= 1; i++) {
		for (j = 0; j < 6; j += 2) {
			if ((connected_mask >> (i + j)) & 1) {
				if ((i + j) == 2 || (i + j) == 5)
					SDL_RenderCopyEx(renderer, textures[i + j], NULL, &draw_rect,
									 0, NULL, SDL_FLIP_HORIZONTAL);
				else
					SDL_RenderCopy(renderer, textures[i + j], NULL, &draw_rect);
			}
		}
	}
}
