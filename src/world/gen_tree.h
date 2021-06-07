#ifndef WORLD_GENERATE_TREE_H
#define WORLD_GENERATE_TREE_H

#include "../procgen/l_system.h"
#include "../lib/vector.h"

typedef struct world_t world_t;

/*
 * tree placement steps:
 * 1. create l_system
 * 2. generate list of locations for trees to be placed
 * 3. call tree_generate with those locations
 */
l_system_t *tree_gen_l_system_create();

void tree_generate(world_t *, l_system_t *lsys, v3i loc, int iterations);

#endif
