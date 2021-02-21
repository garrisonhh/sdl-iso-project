#ifndef VECTOR_H
#define VECTOR_H

#include <stdbool.h>
#include <SDL2/SDL.h>

typedef SDL_Point vector2; // for consistency

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

vector2 vector3ToIsometric(vector3, bool);
vector2 dvector3ToIsometric(dvector3, bool);
vector2 vector2Add(vector2, vector2);
vector2 vector2Sub(vector2, vector2);
vector3 vector3FromDvector3(dvector3);
vector3 vector3Add(vector3, vector3);
dvector3 dvector3FromVector3(vector3);
dvector3 dvector3Add(dvector3, dvector3);
dvector3 dvector3Scale(dvector3, double);
int flatten(vector3, int);
double dvector2DotProd(dvector2, dvector2);

#endif
