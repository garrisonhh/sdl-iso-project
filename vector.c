#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h> // apparently also requires manually linking math.h with -lm compiler flag, what the fuck?
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

dvector2 *perlinVectors;
SDL_Point perlinDims;
int perlinSize;

void vector3ToIsometric(SDL_Point *dest, vector3 *src, int scaleX, int scaleY, int offsetX, int offsetY) {
	dest->x = ((src->x - src->y) * scaleX) >> 1;
	dest->y = ((((src->x + src->y) * scaleY) >> 1) - (src->z * scaleY)) >> 1;
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
	return a + (b - a) * ((1 - cos(x * M_PI)) / 2);
}

double lerp2d(double corners[4], dvector2 *point) {
	return lerp(lerp(corners[0], corners[1], point->x), lerp(corners[2], corners[3], point->x), point->y);
}

void initNoise(unsigned int seed, int w, int h) {
	srand(seed);
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
