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

struct path_network_t {
	// nodes is {v3i : path_node_t *}
	// ids is {v3i : group id}
	hashmap_t *nodes, *ids;
	int groups;
};
typedef struct path_network_t path_network_t;

path_network_t *path_generate_world_network(world_t *);

#endif
