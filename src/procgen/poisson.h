#ifndef POISSON_H
#define POISSON_H

#include "../lib/vector.h"
#include "../lib/array.h"

// return array of malloc'd v2i *
array_t *poisson_samples2(int width, int height, double radius, int samples_per_pt);

#endif
