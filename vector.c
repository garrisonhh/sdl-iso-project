#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "vector.h"
#include "render.h"

/* 
 * apparently M_PI isn't always defined in math.h between c versions?
 * gcc defines M_PI anyways, but it isn't the standard and that
 * would break using another compiler. the c language dude.
 */
#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

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

dvector2 *perlinVectors;
SDL_Point perlinDims;
int perlinSize;

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

int flatten(vector3 *v, int size) {
	return ((v->z * size) + v->y) * size + v->x;
}

double dotProd(dvector2 *a, dvector2 *b) {
	return a->x * b->x + a->y * b->y;
}

// 0 <= x <= 1
double lerp(double a, double b, double x) {
	return a + (b - a) * ((1 - cosl(x * M_PI)) / 2);
}

double lerp2d(double corners[4], dvector2 *point) {
	return lerp(lerp(corners[0], corners[1], point->x), lerp(corners[2], corners[3], point->x), point->y);
}

void initNoise(unsigned int seed, int w, int h) {
	srand(seed); // TODO move this somewhere else for transparency?
	perlinDims.x = w;
	perlinDims.y = h;
	perlinSize = (h + 1) * (w + 1);
	perlinVectors = (dvector2 *)malloc(sizeof(dvector2) * perlinSize);
	for (int i = 0; i < perlinSize; i++) {
		perlinVectors[i].x = (double)(2 * (rand() % 2) - 1);
		perlinVectors[i].y = (double)(2 * (rand() % 2) - 1);
	}
}

double noise(dvector2 *point) {
	dvector2 *modPt = (dvector2 *)malloc(sizeof(dvector2));
	modPt->x = fmod(point->x, 1);
	modPt->y = fmod(point->y, 1);

	dvector2 temp;
	double dotCorners[4], value;
	int i, j, pX = (int)point->x, pY = (int)point->y;
	for (j = 0; j < 2; j++) {
		for (i = 0; i < 2; i++) {
			temp.x = (i) ? (1 - modPt->x) : (modPt->x);
			temp.y = (j) ? (1 - modPt->y) : (modPt->y);
			dotCorners[j * 2 + i] = dotProd(&temp, &perlinVectors[(pY + j) * (perlinDims.x + 1) + (pX + i)]);
		}
	}
	
	value = lerp2d(dotCorners, modPt);
	free(modPt);
	return value;
}

void quitNoise() {
	free(perlinVectors);
	perlinVectors = NULL;
}
