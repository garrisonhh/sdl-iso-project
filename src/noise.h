#ifndef NOISE_H

#define NOISE_H

#include <stddef.h>
#include "vector.h"

typedef struct noise2_t noise2_t;

void noise_test(void);

noise2_t *noise2_create(size_t side_len, int start_pow2, int octaves, double persist);
void noise2_destroy(noise2_t *);

double noise2_at(noise2_t *noise, int x, int y);

#endif
