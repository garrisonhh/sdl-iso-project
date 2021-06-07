#ifndef WORLD_GENERATE_TREE_H
#define WORLD_GENERATE_TREE_H

#include "../procgen/l_system.h"
#include "../lib/vector.h"

typedef struct world_t world_t;
typedef struct tree_generator_t tree_generator_t;

tree_generator_t *tree_oak_generator();

void tree_generate(world_t *, tree_generator_t *, v3i loc);

#endif
