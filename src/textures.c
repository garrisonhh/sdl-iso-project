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

vox_tex *VOID_VOXEL_TEXTURE;

v2i tex_tex_offset;
SDL_Rect vox_tex_rects[3];
Uint8 vox_tex_shades[] = { // flat shading values (out of 255) for each side t-l-r
	255,
	223,
	191
};

// corresponds to texture_type enum values; used for json parsing

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
	size_t i;
	const char *name;
	size_t *arr_index;
	json_object *tex_arr_obj;
	json_object *current_tex;
	hash_table *tex_type_table;
	size_t num_tex_types = 2;
	texture_type *tex_type_ptr;

	char *tex_type_strings[] = {
		"texture",
		"voxel"
	};

	tex_type_table = hash_table_create(num_tex_types * 1.3 + 1);

	for (i = 0; i < num_tex_types; i++) {
		tex_type_ptr = (texture_type *)malloc(sizeof(texture_type *));
		*tex_type_ptr = (texture_type)i;
		hash_set(tex_type_table, tex_type_strings[i], tex_type_ptr);
	}
	
	tex_arr_obj = json_object_object_get(file_obj, "textures");
	num_textures = json_object_array_length(tex_arr_obj);
	textures = (texture_t **)calloc(num_textures, sizeof(texture_t *));
	texture_table = hash_table_create(num_textures * 1.3 + 1);

	for (i = 0; i < num_textures; i++) {
		char path[50];
		textures[i] = (texture_t *)malloc(sizeof(texture_t));

		current_tex = json_object_array_get_idx(tex_arr_obj, i);
		name = json_object_get_string(json_object_object_get(current_tex, "name"));
		sprintf(path, "assets/%s", name);

		// type
		tex_type_ptr = hash_get(tex_type_table,
				                (char *)json_object_get_string(json_object_object_get(current_tex, "type")));

		if (tex_type_ptr == NULL) {
			printf("unknown texture type for texture \"%s\".\n", name);
			exit(1);
		}

		textures[i]->type = *tex_type_ptr;

		// transparency
		if (textures[i]->type == TEX_VOXELTEXTURE)
			textures[i]->transparent = false;
		else
			textures[i]->transparent = json_object_get_boolean(json_object_object_get(current_tex,
																					  "transparent"));

		// image
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

		// add to indexing hash table
		arr_index = (size_t *)malloc(sizeof(size_t));
		*arr_index = i;
		hash_set(texture_table, (char *)name, arr_index);
	}

	hash_table_deep_destroy(tex_type_table);

	VOID_VOXEL_TEXTURE = textures[texture_index("void")]->voxel_texture;
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

// masks use only the last 3 bits; right-left-top order (corresponding to XYZ)
// void_mask determines sides which will be displayed as void (fully black)
void render_voxel_texture(vox_tex *voxel_texture, v2i pos, uint8_t expose_mask, uint8_t void_mask) {
	uint8_t shade;
	bool exposed, voided;
	SDL_Rect draw_rect;
	vox_tex *cur_texture;

	for (int i = 2; i >= 0; i--) {
		if ((exposed = (expose_mask >> i) & 0x1)
		 || (voided = (void_mask >> i) & 0x1)) {
			draw_rect = vox_tex_rects[i];
			draw_rect.x += pos.x;
			draw_rect.y += pos.y;

			cur_texture = (void_mask >> i) & 1 ? VOID_VOXEL_TEXTURE : voxel_texture;

			if (exposed) {
				shade = vox_tex_shades[i];

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
					SDL_RenderCopyEx(renderer, cur_texture->side, NULL, &draw_rect, 0, NULL, SDL_FLIP_HORIZONTAL);
					break;
			}
		}
	}
}
