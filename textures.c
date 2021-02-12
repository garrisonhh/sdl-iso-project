#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "render.h"
#include "textures.h"

texture_t **textures = NULL;

// TODO TEMPORARY; add json integration or something similar
char *texTexNames[] = {
	"bush",
};
char *voxTexNames[] = {
	"dirt",
	"grass",
};

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
	textures = (texture_t **)malloc(sizeof(texture_t *) * NUM_TEXTURES);
	for (int i = 0; i < NUM_TEXTURES; i++) {
		char path[50];
		sprintf(path, "assets/%s", voxTexNames[i]);

		textures[i] = (texture_t *)malloc(sizeof(texture_t));
		textures[i]->voxelTexture = loadVoxelTexture(path);
		textures[i]->type = TEX_VOXELTEXTURE;
	}
}

void destroyMedia() {
	for (int i = 0; i < NUM_TEXTURES; i++) {
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

