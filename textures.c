#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <json-c/json.h>
#include "render.h"
#include "textures.h"

texture_t **textures = NULL;
int numTextures;

SDL_Texture *loadTexture(char* path) {
	SDL_Texture *newTexture = NULL;
	SDL_Surface *loadedSurface = IMG_Load(path);
	if (loadedSurface == NULL) {
		printf("unable to load image %s:\n%s\n", path, IMG_GetError());
		exit(1);
	}

	newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
	if (newTexture == NULL) {
		printf("unable to create texture from %s:\n%s\n", path, SDL_GetError());
		exit(1);
	}

	SDL_FreeSurface(loadedSurface);
	return newTexture;
}

// loads [base_path]_top.png and [base_path]_side.png
vox_tex* loadVoxelTexture(char *basePath) {
	char topPath[100], sidePath[100];
	strcpy(topPath, basePath);
	strcpy(sidePath, basePath);
	strcat(topPath, "_top.png");
	strcat(sidePath, "_side.png");

	vox_tex* newVoxTex = (vox_tex *)malloc(sizeof(vox_tex));
	newVoxTex->top = loadTexture(topPath);
	newVoxTex->side = loadTexture(sidePath);

	return newVoxTex;
}

void loadMedia() {
	json_object *texFile, *texArrayObj;
	texFile = json_object_from_file("assets/textures.json");
	texArrayObj = json_object_object_get(texFile, "textures");
	numTextures = json_object_array_length(texArrayObj);

	textures = (texture_t **)malloc(sizeof(texture_t *) * numTextures);

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
				textures[i]->texture = loadTexture(texPath);
				break;	
			case TEX_VOXELTEXTURE:
				textures[i]->voxelTexture = loadVoxelTexture(path);
				break;
		}
	}
}

void destroyMedia() {
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

