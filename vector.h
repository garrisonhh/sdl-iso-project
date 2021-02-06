#include <SDL2/SDL.h>

#ifndef VECTOR_H
#define VECTOR_H

typedef struct vector3d {
	int x;
	int y;
	int z;
} vector3;

typedef struct dvector2d {
	double x;
	double y;
} dvector2;

void vector3ToIsometric(SDL_Point *, vector3 *, int, int, int, int);
int flatten(vector3 *, int);
void initNoise(unsigned int, int, int);
double noise(dvector2 *);
void quitNoise(void);

#endif
