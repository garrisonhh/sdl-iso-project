#ifndef NOISE_H

#define NOISE_H

#include "vector.h"

void noise_init(unsigned int, v2i);
double noise_at(v2d);
void noise_quit(void);

#endif
