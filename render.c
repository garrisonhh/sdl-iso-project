#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdint.h>
#include <stdio.h> // debug only
#include "world.h"
#include "render.h"
#include "textures.h"
#include "sprites.h"
#include "entity.h"

// TODO think about ways to refactor this file, has become more just "random
// SDL stuff" rather than "renderer stuff"

SDL_Renderer *renderer = NULL;

const int VOXEL_Z_HEIGHT = VOXEL_HEIGHT - (VOXEL_WIDTH >> 1);
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
Uint8 voxTexShades[] = { // flat shading values (out of 255) for each side t-l-r
	255,
	223,
	191
};

void initRenderer(SDL_Window *window) {
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) {
		printf("renderer could not be created:\n%s\n", SDL_GetError());
		exit(1);
	}

	SDL_SetRenderDrawColor(renderer, 0x20, 0x20, 0x20, 0xFF);
	SDL_RenderSetScale(renderer, 2, 2); // TODO un-hardcode scaling
}

void destroyRenderer() {
	SDL_DestroyRenderer(renderer);
	renderer = NULL;
}

SDL_Texture *loadSDLTexture(char *path) {
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
	printf("successfully loaded texture from %s\n", path);
	return newTexture;
}

void loadMedia() {
	loadTextures();
	loadSprites();
}

void destroyMedia() {
	destroyTextures();
	destroySprites();
}

// technically nothing to do with rendering, maybe move somewhere else?
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
void renderVoxelTexture(vox_tex *voxelTexture, SDL_Point *pos, Uint8 exposeMask) {
	for (int i = 2; i >= 0; i--) {
		if ((exposeMask >> i) & 1) {
			SDL_Rect drawRect = offsetRect(&voxTexRects[i], pos);
			Uint8 shade = voxTexShades[i];
			if (i == 0) {
				SDL_SetTextureColorMod(voxelTexture->top, shade, shade, shade);
			} else {
				SDL_SetTextureColorMod(voxelTexture->side, shade, shade, shade);
			}
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

void renderEntity(entity_t *entity) {
	SDL_Point screenPos;
	dvector3ToIsometric(&screenPos, &entity->pos, SCREEN_WIDTH >> 2, SCREEN_HEIGHT >> 2); // TODO un-hardcode scaling
	sprite_t *sprite = sprites[entity->sprite];
	SDL_Rect drawRect = {
		screenPos.x + sprite->offsetX,
		screenPos.y + sprite->offsetY,
		sprite->w,
		sprite->h
	};
	SDL_RenderCopy(renderer, sprite->texture, NULL, &drawRect);
}

void renderChunk(chunk_t *chunk) {
	int x, y, z, index = 0;
	block_t *block;
	SDL_Point screenPos;
	vector3 blockLoc;
	for (z = 0; z < SIZE; z++) {
		for (y = 0; y < SIZE; y++) {
			for (x = 0; x < SIZE; x++) {
				block = chunk->blocks[index++];
				if (block != NULL && block->exposeMask > 0) {
					blockLoc = (vector3){
						x + chunk->loc.x * SIZE,
						y + chunk->loc.y * SIZE,
						z + chunk->loc.z * SIZE
					};
					vector3ToIsometric(&screenPos, &blockLoc, SCREEN_WIDTH >> 2, SCREEN_HEIGHT >> 2); // TODO un-hardcode scaling
						switch (textures[block->texture]->type) {
							case TEX_TEXTURE:
								; // yes, this semicolon is necessary to compile without errors. I am not joking.
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

	renderEntity(world->player); // TODO entity_t sorting into chunk rendering
}

