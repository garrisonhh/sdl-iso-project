#include <SDL2/SDL.h>
#include "vector.h"

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

void vector3ToIsometric(SDL_Point *dest, vector3 *src, int scaleX, int scaleY, int offsetX, int offsetY) {
	dest->x = ((src->x - src->y) * scaleX) >> 1;
	dest->y = ((((src->x + src->y) * scaleY) >> 1) - (src->z * scaleY)) >> 1;
	dest->x += offsetX;
	dest->y += offsetY;
}

int flatten(vector3 *v, int size) {
	return ((v->x * size) + v->y) * size + v->x;
}

