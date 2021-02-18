#ifndef VECTOR_H
#define VECTOR_H

#include <SDL2/SDL.h>

typedef struct {
	int x;
	int y;
	int z;
} vector3;

typedef struct {
	double x;
	double y;
} dvector2;

typedef struct {
	double x;
	double y;
	double z;
} dvector3;

void vector3ToIsometric(SDL_Point *, vector3 *, int, int);
void dvector3ToIsometric(SDL_Point *, dvector3 *, int, int);
dvector3 dvector3Add(dvector3 *, dvector3 *);
int flatten(vector3 *, int);
double dvector2DotProd(dvector2 *, dvector2 *);

#endif
