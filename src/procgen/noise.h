#ifndef NOISE_H

#define NOISE_H

#include <stddef.h>
#include "../lib/vector.h"

typedef struct noise noise2_t;
typedef struct noise noise3_t;

// 2d
noise2_t *noise2_create(size_t side_len, int start_pow2, int octaves, double persist);
void noise2_destroy(noise2_t *);

double noise2_at(noise2_t *noise, int x, int y);

// 3d
noise3_t *noise3_create(size_t side_len, int start_pow2, int octaves, double persist);
void noise3_destroy(noise3_t *);

double noise3_at(noise3_t *noise, int x, int y, int z);

// any dimension
void noise_map_func(struct noise *, double (*mapped)(double v, int side, int x, int y, int z));

#endif
