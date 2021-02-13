#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdint.h>
#include "world.h"
#include "render.h"
#include "textures.h"

SDL_Renderer *renderer = NULL;

#define VOXEL_WIDTH 32
#define VOXEL_HEIGHT 34
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
}

void destroyRenderer() {
	SDL_DestroyRenderer(renderer);
	renderer = NULL;
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
						; // yes, this semicolon is actually necessary to compile without errors. I am not joking.
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

