#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "world.h"
#include "render.h"

SDL_Renderer *renderer = NULL;
vox_tex **textures;
SDL_Rect voxTexRects[3] = { // used to render sides of vox_tex; t-l-r to correspond with bitmask rshifts
	{-(VOXEL_WIDTH >> 1), -(VOXEL_HEIGHT >> 1), VOXEL_WIDTH, VOXEL_HEIGHT >> 1},
	{-(VOXEL_WIDTH >> 1), -(VOXEL_HEIGHT >> 2), VOXEL_WIDTH >> 1, VOXEL_HEIGHT - (VOXEL_HEIGHT >> 2)},
	{0, -(VOXEL_HEIGHT >> 2), VOXEL_WIDTH >> 1, VOXEL_HEIGHT - (VOXEL_HEIGHT >> 2)},
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

void destroyVoxelTexture(vox_tex *voxelTexture) {
	SDL_DestroyTexture(voxelTexture->top);
	SDL_DestroyTexture(voxelTexture->side);
	voxelTexture->top = NULL;
	voxelTexture->side = NULL;
}

void initRenderer(SDL_Window *window) {
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) {
		printf("renderer could not be created:\n%s\n", SDL_GetError());
		exit(1);
	}

	SDL_SetRenderDrawColor(renderer, 0x20, 0x20, 0x20, 0xFF);
}

void destroyRenderer() {
	SDL_DestroyRenderer(renderer);
	renderer = NULL;
}

void loadMedia() {
	textures = (vox_tex **)malloc(sizeof(vox_tex *) * NUM_TEXTURES);
	for (int i = 0; i < NUM_TEXTURES; i++) {
		char path[50];
		sprintf(path, "assets/%i", i);
		textures[i] = loadVoxelTexture(path);
	}
}

void destroyMedia() {
	for (int i = 0; i < NUM_TEXTURES; i++) {
		destroyVoxelTexture(textures[i]);
		textures[i] = NULL;
	}
	free(textures);
	textures = NULL;
}

SDL_Rect offsetRect(SDL_Rect *rect, SDL_Point *offset) {
	SDL_Rect newRect = {
		offset->x + rect->x,
		offset->y + rect->y,
		rect->w,
		rect->h
	};
	return newRect;
}

// exposeMask uses only the last 3 bits; right-left-top order (corresponding to XYZ)
void renderVoxelTexture(vox_tex *voxelTexture, SDL_Point *pos, unsigned char exposeMask) {
	for (int i = 2; i >= 0; i--) {
		if ((exposeMask >> i) & 1) {
			SDL_Rect draw = offsetRect(&voxTexRects[i], pos);
			switch (i) {
			case 0:
				SDL_RenderCopy(renderer, voxelTexture->top, NULL, &draw);
				break;
			case 1:
				SDL_RenderCopy(renderer, voxelTexture->side, NULL, &draw);
				break;
			case 2:
				SDL_RenderCopyEx(renderer, voxelTexture->side, NULL, &draw, 0, NULL, SDL_FLIP_HORIZONTAL);
				break;
			}
		}
	}
}

void renderChunk(chunk_t *chunk) {
	int x, y, z, index = 0;
	block_t *block;
	vector3 pos;
	SDL_Point pt;
	for (z = 0; z < SIZE; z++) {
		for (y = 0; y < SIZE; y++) {
			for (x = 0; x < SIZE; x++) {
				block = chunk->blocks[index++];
				if (block != NULL && block->exposeMask > 0) {
					pos.x = x + chunk->loc.x * SIZE;
					pos.y = y + chunk->loc.y * SIZE;
					pos.z = z + chunk->loc.z * SIZE;
					vector3ToIsometric(&pt, &pos,
									   VOXEL_WIDTH, VOXEL_HEIGHT,
									   SCREEN_WIDTH >> 1, SCREEN_HEIGHT >> 1);
					renderVoxelTexture(textures[block->texture], &pt, block->exposeMask);
				}
			}
		}
	}
}

void renderWorld(world_t *world) {
	for (int i = 0; i < world->numChunks; i++) {
		renderChunk(world->chunks[i]);
	}
}

