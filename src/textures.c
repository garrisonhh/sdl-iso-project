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

v2i tex_tex_offset;
SDL_Rect vox_tex_rects[3];
Uint8 vox_tex_shades[] = { // flat shading values (out of 255) for each side t-l-r
	255,
	223,
	191
};

// workaround for C's weird global constant rules
void textures_init() {
	tex_tex_offset = (v2i){
		-(VOXEL_WIDTH >> 1),
		-VOXEL_Z_HEIGHT
	};

	SDL_Rect vox_tex_rects_tmp[3] = {
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

	memcpy(vox_tex_rects, vox_tex_rects_tmp, sizeof vox_tex_rects_tmp);
}

void textures_load(json_object *file_obj) {
	json_object *tex_arr_obj;
	tex_arr_obj = json_object_object_get(file_obj, "textures");
	num_textures = json_object_array_length(tex_arr_obj);

	json_object *current_png;
	const char *name;
	size_t *arr_index;

	textures = (texture_t **)calloc(num_textures, sizeof(texture_t *));
	texture_table = hash_table_create(num_textures * 1.3 + 1);

	for (size_t i = 0; i < num_textures; i++) {
		char path[50];

		current_png = json_object_array_get_idx(tex_arr_obj, i);
		name = json_object_get_string(json_object_object_get(current_png, "name"));
		sprintf(path, "assets/%s", name);

		textures[i] = (texture_t *)malloc(sizeof(texture_t));
		textures[i]->type = json_object_get_int(json_object_object_get(current_png, "type"));
		textures[i]->transparent = json_object_get_boolean(json_object_object_get(current_png, "transparent"));

		switch (textures[i]->type) {
			case TEX_TEXTURE:;
				char tex_path[54];
				sprintf(tex_path, "%s.png", path);
				textures[i]->texture = load_sdl_texture(tex_path);
				break;	
			case TEX_VOXELTEXTURE:
				textures[i]->voxel_texture = load_voxel_texture(path);
				break;
		}

		arr_index = (size_t *)malloc(sizeof(size_t));
		*arr_index = i;
		hash_set(texture_table, (char *)name, arr_index);
	}
}

void textures_destroy() {
	for (size_t i = 0; i < num_textures; i++) {
		switch (textures[i]->type) {
			case TEX_TEXTURE:
				SDL_DestroyTexture(textures[i]->texture);
				break;
			case TEX_VOXELTEXTURE:
				SDL_DestroyTexture(textures[i]->voxel_texture->top);
				SDL_DestroyTexture(textures[i]->voxel_texture->side);
				free(textures[i]->voxel_texture);
				break;
		}

		free(textures[i]);
	}

	free(textures);
	textures = NULL;
	hash_table_deep_destroy(texture_table);
	texture_table = NULL;
}

size_t texture_index(char *key) {
	void *value;

	if ((value = hash_get(texture_table, key)) != NULL)
		return *(size_t *)value;

	printf("key not found in texture_table: %s\n", key);
	exit(1);
}

void render_tex_texture(SDL_Texture *texture, v2i pos) {
	SDL_Rect draw_rect = {
		pos.x + tex_tex_offset.x,
		pos.y + tex_tex_offset.y,
		VOXEL_WIDTH,
		VOXEL_HEIGHT
	};
	SDL_RenderCopy(renderer, texture, NULL, &draw_rect);
}

// loads [base_path]_top.png and [base_path]_side.png
vox_tex* load_voxel_texture(char *base_path) {
	char top_path[100], side_path[100];

	strcpy(top_path, base_path);
	strcpy(side_path, base_path);
	strcat(top_path, "_top.png");
	strcat(side_path, "_side.png");

	vox_tex* new_vox_tex = (vox_tex *)malloc(sizeof(vox_tex));
	new_vox_tex->top = load_sdl_texture(top_path);
	new_vox_tex->side = load_sdl_texture(side_path);

	return new_vox_tex;
}

// expose_mask uses only the last 3 bits; right-left-top order (corresponding to XYZ)
void render_voxel_texture(vox_tex *voxel_texture, v2i pos, uint8_t expose_mask) {
	SDL_Rect draw_rect;
	uint8_t shade;

	for (int i = 2; i >= 0; i--) {
		if ((expose_mask >> i) & 1) {
			draw_rect = vox_tex_rects[i];
			draw_rect.x += pos.x;
			draw_rect.y += pos.y;
			shade = vox_tex_shades[i];

			if (i == 0)
				SDL_SetTextureColorMod(voxel_texture->top, shade, shade, shade);
			else
				SDL_SetTextureColorMod(voxel_texture->side, shade, shade, shade);

			switch (i) {
				case 0:
					SDL_RenderCopy(renderer, voxel_texture->top, NULL, &draw_rect);
					break;
				case 1:
					SDL_RenderCopy(renderer, voxel_texture->side, NULL, &draw_rect);
					break;
				case 2:
					SDL_RenderCopyEx(renderer, voxel_texture->side, NULL, &draw_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
					break;
			}
		}
	}
}
