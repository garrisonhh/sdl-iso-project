#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "world.h"
#include "render.h"

#define VOXEL_WIDTH 32
#define VOXEL_HEIGHT 36
#define NUM_TEXTURES 5
const int VOXEL_Z_HEIGHT = VOXEL_HEIGHT - (VOXEL_WIDTH >> 1);

SDL_Renderer *renderer = NULL;
texture_t **textures;
SDL_Point texTexOffset = {
	-(VOXEL_WIDTH >> 1),
	-VOXEL_Z_HEIGHT
};
SDL_Rect voxTexRects[3] = { // used to render sides of vox_tex; t-l-r to correspond with bitmask rshifts
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
	textures = (texture_t **)malloc(sizeof(texture_t *) * NUM_TEXTURES);
	for (int i = 0; i < NUM_TEXTURES; i++) {
		char path[50];
		sprintf(path, "assets/%i", i);

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
// is this stupid? I think this is pretty fucking stupid
void renderVoxelTexture(vox_tex *voxelTexture, SDL_Point *pos, unsigned char exposeMask) {
	for (int i = 2; i >= 0; i--) {
		if ((exposeMask >> i) & 1) {
			SDL_Rect drawRect = offsetRect(&voxTexRects[i], pos);
			switch (i) {
			case 0:
				SDL_RenderCopy(renderer, voxelTexture->top, NULL, &drawRect);
				break;
			case 1:
				SDL_RenderCopy(renderer, voxelTexture->side, NULL, &drawRect);
				break;
			case 2:
				SDL_RenderCopyEx(renderer, voxelTexture->side, NULL, &drawRect, 0, NULL, SDL_FLIP_HORIZONTAL);
				break;
			}
		}
	}
}

void renderChunk(chunk_t *chunk) {
	int x, y, z, index = 0;
	block_t *block;
	SDL_Point screenPos;
	for (z = 0; z < SIZE; z++) {
		for (y = 0; y < SIZE; y++) {
			for (x = 0; x < SIZE; x++) {
				block = chunk->blocks[index++];
				if (block != NULL && block->exposeMask > 0) {
					vector3 blockLoc = {
						x + chunk->loc.x * SIZE,
						y + chunk->loc.y * SIZE,
						z + chunk->loc.z * SIZE
					};
					vector3ToIsometric(&screenPos, &blockLoc,
									   VOXEL_WIDTH, VOXEL_Z_HEIGHT,
									   SCREEN_WIDTH >> 1, SCREEN_HEIGHT >> 1);
					switch (textures[block->texture]->type) {
					case TEX_TEXTURE:
						SDL_Rect drawRect = {
							screenPos.x + texTexOffset.x,
							screenPos.y + texTexOffset.y,
							VOXEL_WIDTH,
							VOXEL_HEIGHT
						};
						SDL_RenderCopy(renderer, textures[block->texture]->texture, NULL, &drawRect);
						break;
					case TEX_VOXELTEXTURE:
						renderVoxelTexture(textures[block->texture]->voxelTexture, &screenPos, block->exposeMask);
						break;
					}
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

