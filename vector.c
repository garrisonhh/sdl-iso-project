#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
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

vector2 vector3ToIsometric(vector3 v, bool atCamera) {
	vector2 iso = {
		((v.x - v.y) * VOXEL_WIDTH) >> 1,
		(((v.x + v.y) * VOXEL_WIDTH) >> 2) - (v.z * VOXEL_Z_HEIGHT)
	};

	if (atCamera)
		return vector2Add(iso, camera);
	return iso;
}

vector2 dvector3ToIsometric(dvector3 v, bool atCamera) {
	vector2 iso = {
		((v.x - v.y) * VOXEL_WIDTH) / 2,
		(((v.x + v.y) * VOXEL_WIDTH) / 4) - (v.z * VOXEL_Z_HEIGHT)
	};

	if (atCamera)
		return vector2Add(iso, camera);
	return iso;
}

vector2 vector2Add(vector2 a, vector2 b) {
	return (vector2){
		a.x + b.x,
		a.y + b.y
	};
}

// a - b
vector2 vector2Sub(vector2 a, vector2 b) {
	return (vector2){
		a.x - b.x,
		a.y - b.y
	};
}

vector3 vector3FromDvector3(dvector3 v) {
	return (vector3){
		(int)v.x,
		(int)v.y,
		(int)v.z
	};
}

vector3 vector3Add(vector3 a, vector3 b) {
	return (vector3){
		a.x + b.x,
		a.y + b.y,
		a.z + b.z
	};
}

dvector3 dvector3FromVector3(vector3 v) {
	return (dvector3){
		v.x,
		v.y,
		v.z
	};
}

dvector3 dvector3Add(dvector3 a, dvector3 b) {
	return (dvector3){
		a.x + b.x,
		a.y + b.y,
		a.z + b.z
	};
}

dvector3 dvector3Scale(dvector3 v, double scalar) {
	return (dvector3){
		v.x * scalar,
		v.y * scalar,
		v.z * scalar
	};
}

int flatten(vector3 v, int size) {
	return ((v.z * size) + v.y) * size + v.x;
}

double dvector2DotProd(dvector2 a, dvector2 b) {
	return a.x * b.x + a.y * b.y;
}
