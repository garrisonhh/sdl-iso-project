#include <SDL2/SDL.h>

#ifndef VECTOR_H
#define VECTOR_H

typedef struct vector3d {
	int x;
	int y;
	int z;
} vector3;

void vector3ToIsometric(SDL_Point *, vector3 *, int, int, int, int);
int flatten(vector3 *, int);

#endif
