#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "vector.h"
#include "render.h"

v2i v2i_from_v2d(v2d v) {
	return (v2i){
		(int)v.x,
		(int)v.y
	};
}

v2i v2i_add(v2i a, v2i b) {
	return (v2i){
		a.x + b.x,
		a.y + b.y
	};
}

v2i v2i_sub(v2i a, v2i b) {
	return (v2i){
		a.x - b.x,
		a.y - b.y
	};
}

double v2d_dot(v2d a, v2d b) {
	return a.x * b.x + a.y * b.y;
}

v3i v3i_from_v3d(v3d v) {
	return (v3i){
		(int)v.x,
		(int)v.y,
		(int)v.z
	};
}

int v3i_get(v3i *v, int index) {
	if (index == 0)
		return v->x;
	else if (index == 1)
		return v->y;
	return v->z;
}

void v3i_set(v3i *v, int index, int value) {
	if (index == 0)
		v->x = value;
	else if (index == 1)
		v->y = value;
	else
		v->z = value;
}

v3i v3i_add(v3i a, v3i b) {
	return (v3i){
		a.x + b.x,
		a.y + b.y,
		a.z + b.z
	};
}

v3i v3i_sub(v3i a, v3i b) {
	return (v3i){
		a.x - b.x,
		a.y - b.y,
		a.z - b.z
	};
}

v3i v3i_scale(v3i v, double scalar) {
	return (v3i){
		v.x * scalar,
		v.y * scalar,
		v.z * scalar
	};
}

int v3i_compare(v3i a, v3i b) {
	return ((((a.z - b.z) << 1) + (a.y - b.y)) << 1) + a.x - b.x;
}

v3i polarity_of_v3d(v3d v) {
	v3i polarity;

	for (int i = 0; i < 3; i++)
		v3i_set(&polarity, i, (v3d_get(&v, i) >= 0 ? 1 : -1));

	return polarity;
}

double v3i_magnitude(v3i v) {
	return sqrt(v3i_dot(v, v));
}

double v3i_dot(v3i a, v3i b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

void v3i_print(const char *message, v3i v) {
	if (message != NULL)
		printf("%s:\t", message);
	printf("{%d\t%d\t%d}\n", v.x, v.y, v.z);
}

v3d v3d_from_v3i(v3i v) {
	return (v3d){v.x, v.y, v.z};
}

double v3d_get(v3d *v, int index) {
	if (index == 0)
		return v->x;
	else if (index == 1)
		return v->y;
	return v->z;
}

void v3d_set(v3d *v, int index, double value) {
	if (index == 0)
		v->x = value;
	else if (index == 1)
		v->y = value;
	else
		v->z = value;
}

v3d v3d_add(v3d a, v3d b) {
	return (v3d){
		a.x + b.x,
		a.y + b.y,
		a.z + b.z
	};
}

v3d v3d_sub(v3d a, v3d b) {
	return (v3d){
		a.x - b.x,
		a.y - b.y,
		a.z - b.z
	};
}

v3d v3d_mul(v3d a, v3d b) {
	return (v3d){
		a.x * b.x,
		a.y * b.y,
		a.z * b.z
	};
}

v3d v3d_scale(v3d v, double scalar) {
	return (v3d){
		v.x * scalar,
		v.y * scalar,
		v.z * scalar
	};
}

v3d v3d_normalize(v3d v) {
	return v3d_scale(v, 1 / v3d_magnitude(v));
}

double v3d_magnitude(v3d v) {
	return sqrt(v3d_dot(v, v));
}

double v3d_dist(v3d a, v3d b) {
	v3d diff = v3d_sub(a, b);

	return sqrt(v3d_dot(diff, diff));
}

double v3d_dot(v3d a, v3d b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

void v3d_print(const char *message, v3d v) {
	if (message != NULL)
		printf("%s:\t", message);
	printf("{%9.4f\t%9.4f\t%9.4f}\n", v.x, v.y, v.z);
}
