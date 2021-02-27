#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <json-c/json.h>
#include "render.h"
#include "textures.h"

texture_t **textures = NULL;
int num_textures;

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

void textures_load() {
	json_object *tex_arr_obj;
	tex_arr_obj = json_object_object_get(json_object_from_file("assets/assets.json"), "textures");
	num_textures = json_object_array_length(tex_arr_obj);
	textures = (texture_t **)calloc(num_textures, sizeof(texture_t *));

	for (int i = 0; i < num_textures; i++) {
		json_object *current_png;
		char path[50];
		current_png = json_object_array_get_idx(tex_arr_obj, i);
		sprintf(path, "assets/%s", json_object_get_string(json_object_object_get(current_png, "name")));

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
	}
}

void textures_destroy() {
	for (int i = 0; i < num_textures; i++) {
		switch (textures[i]->type) {
			case TEX_TEXTURE:
				SDL_DestroyTexture(textures[i]->texture);
				break;
			case TEX_VOXELTEXTURE:
				SDL_DestroyTexture(textures[i]->voxel_texture->top);
				SDL_DestroyTexture(textures[i]->voxel_texture->side);
				textures[i]->voxel_texture->top = NULL;
				textures[i]->voxel_texture->side = NULL;
				break;
		}
		free(textures[i]);
		textures[i] = NULL;
	}
	free(textures);
	textures = NULL;
}

