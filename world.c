#include <stdlib.h>
#include <stdio.h>
#include "world.h"

/*
in world.*: composite types holding world data and the functions that manage/create/destroy them
*/

float lerp(float a, float b, float x) {
	return a + (b - a) * x;
}

chunk_t *randomTestChunk(vector3 loc) {
	// quick flat terrain lerp hmap
	int i, j, terrain[256];
	float a, b, corners[4];
	for (i = 0; i < 4; i++) {
		corners[i] = rand() % 17;
	}
	for (i = 0; i < 16; i++) {
		a = lerp(corners[0], corners[1], (float)i / 15);
		b = lerp(corners[2], corners[3], (float)i / 15);
		for (j = 0; j < 16; j++) {
			terrain[i * 16 + j] = (int)(lerp(a, b, (float)j / 15));
		}
	}

	// convert hmap to chunk
	chunk_t *chunk = (chunk_t *)malloc(sizeof(chunk_t));
	int x, y, z, index = 0;
	for (z = 0; z < 16; z++) {
		for (y = 0; y < 16; y++) {
			for (x = 0; x < 16; x++) {
				if (z <= terrain[y * 16 + x]) {
					block_t *block = (block_t *)malloc(sizeof(block_t));
					block->texture = z % 4 + 1;
					chunk->blocks[index] = block;
				} else {
					chunk->blocks[index] = NULL;
				}
				index++;
			}
		}
	}
	chunk->loc = loc;
	return chunk;
}

void destroyChunk(chunk_t *chunk) {
	for (int i = 0; i < 4096; i++) {
		if (chunk->blocks[i] != NULL) {
			free(chunk->blocks[i]);
			chunk->blocks[i] = NULL;
		}
	}
	free(chunk);
}
