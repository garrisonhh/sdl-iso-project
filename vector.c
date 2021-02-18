#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include "vector.h"
#include "render.h"

/*
   z
   |
   |
   .
  / \
 /   \
y     x
in vector.*: functions dealing with point math and 3d/2d conversion
*/

void vector3ToIsometric(SDL_Point *dest, vector3 *src, int offsetX, int offsetY) {
	dest->x = ((src->x - src->y) * VOXEL_WIDTH) >> 1;
	// this can't be shortened, stop trying
	dest->y = (((src->x + src->y) * VOXEL_WIDTH) >> 2) - (src->z * VOXEL_Z_HEIGHT);
	dest->x += offsetX;
	dest->y += offsetY;
}

void dvector3ToIsometric(SDL_Point *dest, dvector3 *src, int offsetX, int offsetY) {
	dest->x = ((src->x - src->y) * VOXEL_WIDTH) / 2;
	dest->y = (((src->x + src->y) * VOXEL_WIDTH) / 4) - (src->z * VOXEL_Z_HEIGHT);
	dest->x += offsetX;
	dest->y += offsetY;
}

dvector3 dvector3Add(dvector3 *a, dvector3 *b) {
	return (dvector3){a->x + b->x, a->y + b->y, a->z + b->z};
}

int flatten(vector3 *v, int size) {
	return ((v->z * size) + v->y) * size + v->x;
}

double dvector2DotProd(dvector2 *a, dvector2 *b) {
	return a->x * b->x + a->y * b->y;
}
