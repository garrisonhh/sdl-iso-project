#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "vector.h"
#include "render.h"

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

v3i v3i_scale(v3i v, int scalar) {
	return (v3i){
		v.x * scalar,
		v.y * scalar,
		v.z * scalar
	};
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

v3d v3d_scale(v3d v, double scalar) {
	return (v3d){
		v.x * scalar,
		v.y * scalar,
		v.z * scalar
	};
}
