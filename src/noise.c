#include <stdlib.h>
#include <math.h>
#include "noise.h"
#include "vector.h"
#include "utils.h"

#include <stdio.h> // TODO REMOVE

const v2d RANDOM_VECTORS[4] = {
	(v2d){ 1,  1},
	(v2d){ 1, -1},
	(v2d){-1,  1},
	(v2d){-1, -1},
};

struct layer2_t {
	v2d *vectors;
	size_t side, size;
};
typedef struct layer2_t layer2_t;

struct noise2_t {
	double *map;
	size_t side, size;
};
typedef struct noise2_t noise2_t;

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
		layer->vectors[i] = RANDOM_VECTORS[rand() % 4];

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

	for (offset.y = 0; offset.y <= 1; ++offset.y) {
		for (offset.x = 0; offset.x <= 1; ++offset.x) {
			rel = (v2d){
				(offset.x ? 1.0 - pos.x : pos.x),
				(offset.y ? 1.0 - pos.y : pos.y),
			};

			corner = layer->vectors[(posi.y + offset.y) * layer->side + (posi.x + offset.x)];
			corners[(offset.y << 1) | offset.x] = v2d_dot(rel, corner);
		}
	}

	return lerp2(corners, pos);
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

	for (y = 0; y < noise->side; ++y)
		for (x = 0; x < noise->side; ++x)
			noise->map[(y * noise->side) + x] = 0;

	for (i = 0; i < octaves; ++i) {
		layer = noise_gen_layer2(i + start_pow2);

		for (y = 0; y < noise->side; ++y) {
			for (x = 0; x < noise->side; ++x) {
				pos.x = (double)x / (double)(noise->side - 1);
				pos.y = (double)y / (double)(noise->side - 1);

				noise->map[(y * noise->side) + x] += layer2_at(layer, pos) * persistence;
			}
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

void noise_test() {
	int x, y;
	int side = 50;
	double v;

	for (int i = 0; i < 10; ++i) {
		noise2_t *noise = noise2_create(side, 2, 3, 0.3);

		double minv = 1.0, maxv = -1.0;

		for (y = 0; y < side; ++y) {
			for (x = 0; x < side; ++x) {
				v = noise2_at(noise, x, y);

				if (v < minv)
					minv = v;
				if (v > maxv)
					maxv = v;

				term_set_bg((int)(((v + 1.0) / 2.0) * 255));
				printf("  ");
			}
			term_reset_color();
			printf("\n");
		}

		printf("max: %f min: %f\n", maxv, minv);

		noise2_destroy(noise);
	}
}

