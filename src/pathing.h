#ifndef PATHING_H
#define PATHING_H

#include <stdbool.h>
#include "lib/vector.h"
#include <ghh/hashmap.h>
#include <ghh/list.h>
#include <ghh/array.h>

typedef struct world world_t;

// path network types
typedef struct path_node {
	v3i pos;
	array_t *connects;
} path_node_t;

typedef struct path_connect {
	double weight;
	struct path_node *node;
} path_connect_t;

typedef struct path_network {
	// nodes is {v3i : path_node_t *}
	// ids is {v3i : group id}
	hashmap_t *nodes, *ids;
	// the integers ids points to
	array_t *id_targets;
} path_network_t;

// astar types
enum path_astar_set_e {
	PATH_ASTAR_OPEN,
	PATH_ASTAR_CLOSED
};
typedef enum path_astar_set_e path_astar_set_e;

typedef struct path_asnode {
	double g, h, f;
	v3i pos, prev;
	path_astar_set_e set;
} path_asnode_t;

bool path_block_pathable(world_t *, v3i);
path_network_t *path_generate_world_network(world_t *);
void path_network_destroy(path_network_t *);
list_t *path_find(path_network_t *, v3i start_pos, v3i goal_pos);

#endif
