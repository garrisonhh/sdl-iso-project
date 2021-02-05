#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "vector.h"
#include "world.h"

/*
in this file:
- main()
- anything related to rendering graphics to the screen
*/

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

SDL_Point *camera = NULL; // TODO camera motion

// textures for voxels
typedef struct voxel_texture {
	SDL_Texture* top;
	SDL_Texture* side; // use SDL_RenderCopyEx to render flipped
} vox_tex;

#define VOXEL_WIDTH 32
#define VOXEL_HEIGHT 32
const int numTextures = 5;
vox_tex *textures[5];

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

// exposeMask uses only the last 3 bits; right-left-top order (corresponding to XYZ)
// the weird magic pragma shit suppresses an incorrect warning thats bugging the fuck out of me
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
void renderVoxelTexture(vox_tex *voxelTexture, SDL_Point *pos, unsigned char exposeMask) {
	SDL_Rect draw;
	if ((exposeMask >> 2) & 1) { // right
		draw.x = pos->x;
		draw.y = pos->y + (VOXEL_HEIGHT >> 2);
		draw.w = VOXEL_WIDTH >> 1;
		draw.h = VOXEL_HEIGHT - (VOXEL_HEIGHT >> 2);
		SDL_RenderCopyEx(renderer, voxelTexture->side, NULL, &draw, 0, NULL, SDL_FLIP_HORIZONTAL);
	}
	if ((exposeMask >> 1) & 1) { // left
		draw.x = pos->x - (VOXEL_WIDTH >> 1);
		draw.y = pos->y + (VOXEL_HEIGHT >> 2);
		draw.w = VOXEL_WIDTH >> 1;
		draw.h = VOXEL_HEIGHT - (VOXEL_HEIGHT >> 2);
		SDL_RenderCopy(renderer, voxelTexture->side, NULL, &draw);
	}
	if (exposeMask & 1) { // top
		draw.x = pos->x - (VOXEL_WIDTH >> 1);
		draw.y = pos->y;
		draw.w = VOXEL_WIDTH;
		draw.h = VOXEL_HEIGHT >> 1;
		SDL_RenderCopy(renderer, voxelTexture->top, NULL, &draw);
	}
}
#pragma GCC diagnostic pop

// TODO dynamically update block exposure based on surrounding block updates
void exposeChunk(chunk_t *chunk) {
	block_t *block;
	block_t *other;
	int i, j, offset;
	unsigned char newMask;
	for (i = 0; i < CHUNK_SIZE; i++) {
		block = chunk->blocks[i];
		if (block != NULL) {
			newMask = 0;
			offset = 1;
			for (j = 0; j < 3; j++) {
				if ((i + offset) / (offset * SIZE) > i / (offset * SIZE)) {
					newMask |= 1;
				} else {
					other = chunk->blocks[i + offset];
					if (other == NULL) {
						newMask |= 1;
					}
				}
				if (j < 2) {
					offset *= SIZE;
					newMask <<= 1;
				}
			}
			block->exposeMask = newMask;
		}
	}
}

void exposeWorld(world_t *world) {
	for (int i = 0; i < world->numChunks; i++) {
		exposeChunk(world->chunks[i]);
	}
}

// TODO render chunks at correct location, camera offset, etc
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

void init() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize:\n%s\n", SDL_GetError());
		exit(1);
	}

	// window
	window = SDL_CreateWindow("render this bitch",
							  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT,
							  SDL_WINDOW_SHOWN);
	if (window == NULL) {
		printf("window could not be created:\n%s\n", SDL_GetError());
		exit(1);
	}
	
	// hw accel renderer
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) {
		printf("renderer could not be created:\n%s\n", SDL_GetError());
		exit(1);
	}

	SDL_SetRenderDrawColor(renderer, 0x20, 0x20, 0x20, 0xFF);

	// png loading
	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags)) {
		printf("SDL_image could not initialize:\n%s\n", IMG_GetError());
		exit(1);
	}
}

void loadMedia() {
	for (int i = 0; i < numTextures; i++) {
		char path[50];
		sprintf(path, "assets/%i", i);
		textures[i] = loadVoxelTexture(path);
	}
}

// naming this "close" results in seg fault lmao. func name conflict in sdl somewhere?
void onClose() {
	for (int i = 0; i < numTextures; i++) {
		destroyVoxelTexture(textures[i]);
		textures[i] = NULL;
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	renderer = NULL;
	window = NULL;

	IMG_Quit();
	SDL_Quit();
}

int main() {
	init();
	loadMedia();

	vector3 dims = {4, 4, 2};
	world_t *world = createWorld(dims);
	generateWorld(world);

	unsigned int lastTime, frameTick = 100;
	unsigned int thisTime = SDL_GetTicks();
	float frameRate;
	unsigned int frames[100];

	bool quit = false;
	SDL_Event e;

	while (!quit) {
		while (SDL_PollEvent(&e) != 0) {
			switch (e.type) {
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_KEYDOWN:
				switch (e.key.keysym.sym) { // nothing wrong with this. very normal code.
				case SDLK_ESCAPE:
					quit = true;
					break;
				}
				break;
			}
		}

		exposeWorld(world);
		SDL_RenderClear(renderer);
		renderWorld(world);
		SDL_RenderPresent(renderer);

		lastTime = thisTime;
		thisTime = SDL_GetTicks();
		for (int i = 100; i > 0; i--) {
			frames[i] = frames[i - 1];
		}
		frames[0] = thisTime - lastTime;
		frameTick--;
		if (frameTick == 0) {
			frameRate = 0;
			for (int i = 0; i < 100; i++) {
				frameRate += frames[i];
			}
			frameRate /= 100;
			printf("FPS: %.2f\n", 1000 / frameRate);
			frameTick = 100;
		}
	}

	destroyWorld(world);
	world = NULL;
	onClose();
	return 0;
}
