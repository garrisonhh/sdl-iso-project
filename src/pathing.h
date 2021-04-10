#ifndef PATHING_H
#define PATHING_H

#include "world.h"
#include "vector.h"
#include "data_structures/hashmap.h"

struct path_node_t {
	v3i pos;
	// map of {connected pos : path_connect_t *}
	hashmap_t *connects;
};
typedef struct path_node_t path_node_t;

struct path_connect_t {
	double weight;
	struct path_node_t *node;
};
typedef struct path_connect_t path_connect_t;

hashmap_t *path_map_construct(world_t *);

#endif
