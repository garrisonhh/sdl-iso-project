#ifndef POISSON_H
#define POISSON_H

#include "noise.h"
#include "../lib/vector.h"
#include "../lib/array.h"

// return array of malloc'd v2i *
array_t *poisson2_samples(int width, int height, double radius, int samples_per_pt);

// prune generated samples in order to get variable density distributions
void poisson2_prune_worst(array_t *samples, noise2_t *noise, double percentage);
void poisson2_prune_best(array_t *samples, noise2_t *noise, double percentage);

void poisson2_prune_linear(array_t *samples, noise2_t *noise, double percentage);

#endif
