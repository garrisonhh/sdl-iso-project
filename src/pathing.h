#ifndef PATHING_H
#define PATHING_H

#include <stdbool.h>
#include "world.h"
#include "vector.h"
#include "data_structures/hashmap.h"
#include "data_structures/list.h"
#include "data_structures/array.h"

// path network types
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
	// the integers ids points to
	array_t *id_targets;
};
typedef struct path_network_t path_network_t;

// astar types
enum path_astar_set_e {
	PATH_ASTAR_OPEN,
	PATH_ASTAR_CLOSED
};
typedef enum path_astar_set_e path_astar_set_e;

struct path_asnode_t {
	double g, h, f;
	v3i pos, prev;
	path_astar_set_e set;
};
typedef struct path_asnode_t path_asnode_t;

bool path_block_pathable(world_t *, v3i);
path_network_t *path_generate_world_network(world_t *);
void path_network_destroy(path_network_t *);
list_t *path_find(path_network_t *, v3i start_pos, v3i goal_pos);

#endif
