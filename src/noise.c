#include <math.h>
#include "vector.h"
#include "utils.h"

v2d *perlin_vectors;
v2i perlin_dims;
int perlin_size;

// 0 <= x <= 1
double lerp(double a, double b, double x) {
	return a + (b - a) * ((1 - cosl(x * M_PI)) / 2);
}

double lerp2d(double corners[4], v2d *point) {
	return lerp(lerp(corners[0], corners[1], point->x), lerp(corners[2], corners[3], point->x), point->y);
}

void noise_init(v2i dims) {
	perlin_dims = dims;
	perlin_size = (dims.y + 1) * (dims.x + 1);
	perlin_vectors = (v2d *)malloc(sizeof(v2d) * perlin_size);

	for (int i = 0; i < perlin_size; i++) {
		perlin_vectors[i].x = (double)((rand() % 3) - 1);
		perlin_vectors[i].y = (double)((rand() % 3) - 1);
	}
}

// TODO rewrite this, it's not that complicated just unreadable shitty code
// generates numbers between -1 and 1
double noise_at(v2d point) {
	v2d corner;
	double dot_corners[4];
	int i, j;

	int pX = (int)point.x, pY = (int)point.y;
	v2d mod_pt = {
		fmod(point.x, 1),
		fmod(point.y, 1)
	};

	for (i = 0; i <= 1; i++) {
		for (j = 0; j <= 1; j++) {
			corner.x = i ? 1 - mod_pt.x : mod_pt.x;
			corner.y = j ? 1 - mod_pt.y : mod_pt.y;
			dot_corners[(j << 1) + i] = v2d_dot(
				corner,
				perlin_vectors[(pY + j) * (perlin_dims.x + 1) + (pX + i)]
			);
		}
	}
	
	return lerp2d(dot_corners, &mod_pt);
}

void noise_quit() {
	free(perlin_vectors);
	perlin_vectors = NULL;
}
