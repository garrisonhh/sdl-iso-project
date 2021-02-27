#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <json-c/json.h>
#include "render.h"
#include "textures.h"

texture_t **textures = NULL;
int numTextures;

// loads [base_path]_top.png and [base_path]_side.png
vox_tex* loadVoxelTexture(char *basePath) {
	char topPath[100], sidePath[100];
	strcpy(topPath, basePath);
	strcpy(sidePath, basePath);
	strcat(topPath, "_top.png");
	strcat(sidePath, "_side.png");

	vox_tex* newVoxTex = (vox_tex *)malloc(sizeof(vox_tex));
	newVoxTex->top = loadSDLTexture(topPath);
	newVoxTex->side = loadSDLTexture(sidePath);

	return newVoxTex;
}

void loadTextures() {
	json_object *texArrayObj;
	texArrayObj = json_object_object_get(json_object_from_file("assets/assets.json"), "textures");
	numTextures = json_object_array_length(texArrayObj);
	textures = (texture_t **)calloc(numTextures, sizeof(texture_t *));

	for (int i = 0; i < numTextures; i++) {
		json_object *currentTex;
		char path[50];
		currentTex = json_object_array_get_idx(texArrayObj, i);
		sprintf(path, "assets/%s", json_object_get_string(json_object_object_get(currentTex, "name")));

		textures[i] = (texture_t *)malloc(sizeof(texture_t));
		textures[i]->type = json_object_get_int(json_object_object_get(currentTex, "type"));
		textures[i]->transparent = json_object_get_boolean(json_object_object_get(currentTex, "transparent"));
		switch (textures[i]->type) {
			case TEX_TEXTURE:;
				char texPath[54];
				sprintf(texPath, "%s.png", path);
				textures[i]->texture = loadSDLTexture(texPath);
				break;	
			case TEX_VOXELTEXTURE:
				textures[i]->voxelTexture = loadVoxelTexture(path);
				break;
		}
	}
}

void destroyTextures() {
	for (int i = 0; i < numTextures; i++) {
		switch (textures[i]->type) {
			case TEX_TEXTURE:
				SDL_DestroyTexture(textures[i]->texture);
				break;
			case TEX_VOXELTEXTURE:
				SDL_DestroyTexture(textures[i]->voxelTexture->top);
				SDL_DestroyTexture(textures[i]->voxelTexture->side);
				textures[i]->voxelTexture->top = NULL;
				textures[i]->voxelTexture->side = NULL;
				break;
		}
		free(textures[i]);
		textures[i] = NULL;
	}
	free(textures);
	textures = NULL;
}

