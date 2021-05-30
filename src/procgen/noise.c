#include <stdlib.h>
#include <math.h>
#include "noise.h"
#include "../lib/utils.h"

const v2d RANDOM_VECTORS2[4] = {
	(v2d){ 1,  1},
	(v2d){ 1, -1},
	(v2d){-1,  1},
	(v2d){-1, -1},
};
const v3d RANDOM_VECTORS3[8] = {
	(v3d){ 1,  1,  1},
	(v3d){ 1,  1, -1},
	(v3d){ 1, -1,  1},
	(v3d){ 1, -1, -1},
	(v3d){-1,  1,  1},
	(v3d){-1,  1, -1},
	(v3d){-1, -1,  1},
	(v3d){-1, -1, -1},
};

struct layer2_t {
	v2d *vectors;
	size_t side, size;
};
typedef struct layer2_t layer2_t;

struct layer3_t {
	v3d *vectors;
	size_t side, size;
};
typedef struct layer3_t layer3_t;

struct noise_t {
	double *map;
	size_t side, size;
}; // typedef'd as noise2_t and noise3_t

// 0 <= x <= 1
double lerp(double a, double b, double x) {
	return a + (b - a) * ((1 - cosl(x * M_PI)) / 2);
}

double lerp2(double corners[4], v2d point) {
	return lerp(lerp(corners[0], corners[1], point.x), lerp(corners[2], corners[3], point.x), point.y);
}

double lerp3(double corners[8], v3d point) {
	v2d point2 = {point.x, point.y};
	return lerp(lerp2(corners, point2), lerp2(corners + 4, point2), point.z);
}

layer2_t *noise_gen_layer2(int side_pow2) {
	layer2_t *layer = malloc(sizeof(layer2_t));

	layer->side = (1 << side_pow2) + 1;
	layer->size = layer->side * layer->side;
	layer->vectors = malloc(sizeof(v2d) * layer->size);

	for (size_t i = 0; i < layer->size; ++i)
		layer->vectors[i] = RANDOM_VECTORS2[rand() % 4];

	return layer;
}

void layer2_destroy(layer2_t *layer) {
	free(layer->vectors);
	free(layer);
}

// 0 <= (x and y) <= 1
double layer2_at(layer2_t *layer, v2d pos) {
	v2i posi, offset;
	v2d rel, corner;
	double corners[4];

	pos = v2d_scale(pos, layer->side - 1);
	posi = v2i_from_v2d(pos);
	pos = v2d_sub(pos, v2d_from_v2i(posi));

	for (int i = 0; i <= 1; ++i) {
		if (v2d_IDX(pos, i) < FLOAT_TOLERANCE && v2i_IDX(posi, i) > 0) {
			v2d_IDX(pos, i) = 1.0;
			--v2i_IDX(posi, i);
		}
	}
	
	// posi now is top left corner; pos is now position relative to square

	FOR_XY(offset.x, offset.y, 2, 2) {
		rel = (v2d){
			(offset.x ? 1.0 - pos.x : pos.x),
			(offset.y ? 1.0 - pos.y : pos.y),
		};

		corner = layer->vectors[(posi.y + offset.y) * layer->side + (posi.x + offset.x)];
		corners[(offset.y << 1) | offset.x] = v2d_dot(rel, corner);
	}

	return lerp2(corners, pos);
}

layer3_t *noise_gen_layer3(int side_pow2, int dimensions) {
	layer3_t *layer = malloc(sizeof(layer3_t));

	layer->side = (1 << side_pow2) + 1;
	layer->size = layer->side * layer->side * layer->side;
	layer->vectors = malloc(sizeof(v3d) * layer->size);

	for (size_t i = 0; i < layer->size; ++i)
		layer->vectors[i] = RANDOM_VECTORS3[rand() % 8];

	return layer;
}

void layer3_destroy(layer3_t *layer) {
	free(layer->vectors);
	free(layer);
}

double layer3_at(layer3_t *layer, v3d pos) {
	v3i posi, offset;
	v3d rel, corner;
	double corners[8];
	int index;

	pos = v3d_scale(pos, layer->side - 1);
	posi = v3i_from_v3d(pos);
	pos = v3d_sub(pos, v3d_from_v3i(posi));

	for (int i = 0; i < 3; ++i) {
		if (v3d_IDX(pos, i) < FLOAT_TOLERANCE && v3i_IDX(posi, i) > 0) {
			v3d_IDX(pos, i) = 1.0;
			--v3i_IDX(posi, i);
		}
	}
	
	// posi now is top left corner; pos is now position relative to square

	FOR_XYZ(offset.x, offset.y, offset.z, 2, 2, 2) {
		rel = (v3d){
			(offset.x ? 1.0 - pos.x : pos.x),
			(offset.y ? 1.0 - pos.y : pos.y),
			(offset.z ? 1.0 - pos.z : pos.z),
		};

		index = (posi.z + offset.z) * layer->side + (posi.y + offset.y);
		index = index * layer->side + (posi.x + offset.x);
		corner = layer->vectors[index];
		corners[(offset.z << 2) | (offset.y << 1) | offset.x] = v3d_dot(rel, corner);
	}

	return lerp3(corners, pos);
}

noise2_t *noise2_create(size_t side_length, int start_pow2, int octaves, double persistence) {
	int x, y, i;
	v2d pos;
	double max_val = 0;
	layer2_t *layer;
	noise2_t *noise = malloc(sizeof(noise2_t));

	noise->side = side_length;
	noise->size = noise->side * noise->side;
	noise->map = malloc(sizeof(double) * noise->size);

	FOR_XY(x, y, noise->side, noise->side)
		noise->map[(y * noise->side) + x] = 0;

	for (i = 0; i < octaves; ++i) {
		layer = noise_gen_layer2(i + start_pow2);

		FOR_XY(x, y, noise->side, noise->side) {
			pos.x = (double)x / (double)(noise->side - 1);
			pos.y = (double)y / (double)(noise->side - 1);

			noise->map[(y * noise->side) + x] += layer2_at(layer, pos) * persistence;
		}

		layer2_destroy(layer);

		max_val += persistence;
		persistence *= persistence;
	}

	// scale back to between -1.0 and 1.0
	for (i = 0; i < noise->size; ++i)
		noise->map[i] /= max_val;

	return noise;
}

void noise2_destroy(noise2_t *noise) {
	free(noise->map);
	free(noise);
}

double noise2_at(noise2_t *noise, int x, int y) {
	return noise->map[(y * noise->side) + x];
}

noise3_t *noise3_create(size_t side_length, int start_pow2, int octaves, double persistence) {
	int x, y, z, i;
	v3d pos;
	double max_val = 0;
	layer3_t *layer;
	noise3_t *noise = malloc(sizeof(noise3_t));

	noise->side = side_length;
	noise->size = noise->side * noise->side * noise->side;
	noise->map = malloc(sizeof(double) * noise->size);

	FOR_CUBE(x, y, z, 0, noise->side)
		noise->map[((z * noise->side + y) * noise->side) + x] = 0;

	for (i = 0; i < octaves; ++i) {
		layer = noise_gen_layer3(i + start_pow2, 2);

		FOR_CUBE(x, y, z, 0, noise->side) {
			pos.x = (double)x / (double)(noise->side - 1);
			pos.y = (double)y / (double)(noise->side - 1);
			pos.z = (double)z / (double)(noise->side - 1);

			noise->map[((z * noise->side + y) * noise->side) + x] += layer3_at(layer, pos) * persistence;
		}

		layer3_destroy(layer);

		max_val += persistence;
		persistence *= persistence;
	}

	// scale back to between -1.0 and 1.0
	for (i = 0; i < noise->size; ++i)
		noise->map[i] /= max_val;

	return noise;
}

void noise3_destroy(noise3_t *noise) {
	free(noise->map);
	free(noise);
}

double noise3_at(noise3_t *noise, int x, int y, int z) {
	return noise->map[(((z * noise->side) + y) * noise->side) + x];
}

