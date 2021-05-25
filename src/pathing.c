#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "pathing.h"
#include "vector.h"
#include "data_structures/hashmap.h"
#include "data_structures/list.h"
#include "data_structures/heap.h"
#include "world.h"
#include "block.h"
#include "utils.h"

const double SQRT_2 = sqrt(2);

path_node_t *path_node_create(v3i pos) {
	path_node_t *node = malloc(sizeof(path_node_t));

	node->pos = pos;
	node->connects = hashmap_create(12, HASH_V3I);

	return node;
}

void path_node_destroy(path_node_t *node) {
	hashmap_destroy(node->connects, true);

	free(node);
}

void path_network_destroy(path_network_t *network) {
	hashmap_iter_t *iter = hashmap_iter_create(network->nodes);
	path_node_t *node;

	while ((node = hashmap_iter_next(iter)) != NULL)
		path_node_destroy(node);

	free(iter);

	hashmap_destroy(network->nodes, false);
	hashmap_destroy(network->ids, false); 
	array_destroy(network->id_targets, true);

	free(network);
}

/*
 * heuristic used here are calculated by using the manhatten + diagonal distance on the (x, y)
 * plane, and then any z difference is multiplied by 2. this results in paths that slightly bias
 * towards flat terrain
 */
double path_heuristic(v3i a, v3i b) {
	v3i delta;
	int dmax, dmin, i;

	delta = v3i_sub(a, b);

	for (i = 0; i < 3; i++)
		v3i_IDX(delta, i) = abs(v3i_IDX(delta, i));

	dmax = MAX(delta.x, delta.y);
	dmin = MIN(delta.x, delta.y);

	return (dmin * SQRT_2) + (dmax - dmin) + (delta.z << 1);
}

void path_node_connect(path_node_t *a, path_node_t *b) {
	path_node_t *nodes[2];
	path_connect_t *connect;
	double weight;
	int i;

	nodes[0] = a;
	nodes[1] = b;

	weight = path_heuristic(a->pos, b->pos);

	for (i = 0; i <= 1; i++) {
		connect = malloc(sizeof(path_connect_t));

		connect->weight = weight;
		connect->node = nodes[!i];

		hashmap_set(nodes[i]->connects, &nodes[1 - i]->pos,  connect);
	}
}

bool path_block_empty(world_t *world, v3i loc) {
	block_t *block = world_get(world, loc);
	
	return block == NULL || block->coll_data->coll_type == BLOCK_COLL_NONE;
}

bool path_block_solid(world_t *world, v3i loc) {
	block_t *block = world_get(world, loc);
	
	return block != NULL && block->coll_data->coll_type == BLOCK_COLL_DEFAULT_BOX;
}

bool path_block_pathable(world_t *world, v3i loc) {
	// holy crap its THE COMMA OPERATOR IN ALL OF ITS GLORY
	return path_block_empty(world, loc) && (loc.z--, path_block_solid(world, loc));
}

// **will remove all buckets from nodes**
path_network_t *path_network_from_nodes(hashmap_t *nodes) {
	list_t *active;
	hashmap_iter_t *node_iter, *connect_iter;
	path_network_t *network;
	path_node_t *node, *neighbor;
	int id = 0;
	int *id_val;

	active = list_create();
	node_iter = hashmap_iter_create(nodes);
	neighbor = NULL;

	network = malloc(sizeof(path_network_t));
	network->nodes = hashmap_create(nodes->max_size, HASH_V3I);
	network->ids = hashmap_create(nodes->max_size, HASH_V3I);
	network->id_targets = array_create(2);

	while (nodes->size) {
		hashmap_iter_reset(node_iter);
		node = hashmap_iter_next(node_iter);

		// how to free this properly..?
		id_val = malloc(sizeof(int));
		*id_val = id;
		array_add(network->id_targets, id_val);

		// find all nodes connected to the starting node, remove them from original nodes list
		// and add them to the network with a unique id tied to the group
		if (hashmap_remove(nodes, &node->pos) == NULL) {
			v3i_print("couldn't find", node->pos);

			if (hashmap_get(nodes, &node->pos) != NULL)
				printf("could get it though..?\n");
			else
				printf("couldn't get it either\n");

			exit(1);
		}

		hashmap_set(network->nodes, &node->pos, node);
		hashmap_set(network->ids, &node->pos, id_val);

		list_push(active, node);

		while (active->size) {
			node = list_pop(active);

			if (node->connects->size) {
				connect_iter = hashmap_iter_create(node->connects);

				while ((neighbor = hashmap_iter_next(connect_iter)) != NULL) {
					if (hashmap_get(nodes, &neighbor->pos) != NULL) {
						hashmap_remove(nodes, &neighbor->pos);
						hashmap_set(network->nodes, &neighbor->pos, neighbor);
						hashmap_set(network->ids, &neighbor->pos, id_val);

						list_push(active, neighbor);
					}
				}

				free(connect_iter);
			}
		}

		++id;
	}

	list_destroy(active, true);
	free(node_iter);

	return network;
}

path_network_t *path_generate_world_network(world_t *world) {
	hashmap_t *nodes;
	path_node_t *current, *neighbor;
	path_network_t *network;
	v3i pos, offset, nbor_pos, other1, other2;
	int i, temp;

	nodes = hashmap_create(world->block_size * world->block_size * 2, HASH_V3I);

	FOR_CUBE(pos.x, pos.y, pos.z, 0, world->block_size) {
		if (path_block_pathable(world, pos)) {
			current = path_node_create(pos);
			hashmap_set(nodes, &current->pos, current);

			// find  neighbors in cardinal directions
			offset.x = 1;
			offset.y = 0;

			for (i = 0; i < 4; i++) {
				for (offset.z = 1; offset.z >= -1; offset.z--) {
					nbor_pos = v3i_add(pos, offset);

					if ((neighbor = hashmap_get(nodes, &nbor_pos)) != NULL)
						path_node_connect(current, neighbor);
				}

				SWAP(offset.x, offset.y, temp);
				offset.y = -offset.y;
			}

			// find neighbors in diagonal directions
			// for neighbors at z offsets 1 or 0, they are considered accessible if the blocks
			// to their right and left are empty
			// fore neighbors at z offset -1, they are considered accessible if the blocks at
			// z offset 0 are empty
			offset.x = 1;
			offset.y = 1;

			for (i = 0; i < 4; i++) {
				for (offset.z = 1; offset.z >= -1; offset.z--) {
					nbor_pos = v3i_add(pos, offset);

					if ((neighbor = hashmap_get(nodes, &nbor_pos)) != NULL) {
						other1.x = nbor_pos.x;
						other1.y = pos.y;
						other1.z = (offset.z == -1 ? pos.z : nbor_pos.z);

						other2.x = pos.x;
						other2.y = nbor_pos.y;
						other2.z = other1.z;
						
						if (path_block_empty(world, other1) && path_block_empty(world, other2))
							path_node_connect(current, neighbor);

					}
				}
				
				SWAP(offset.x, offset.y, temp);
				offset.y = -offset.y;
			}
		}
	}

	network = path_network_from_nodes(nodes);

	hashmap_destroy(nodes, false);

	return network;
}

path_asnode_t *path_asnode_create(v3i pos, path_asnode_t *prev, v3i goal_pos) {
	path_asnode_t *asnode = malloc(sizeof(path_asnode_t));

	asnode->pos = pos;

	if (prev != NULL) {
		asnode->prev = prev->pos;
		asnode->g = prev->g + path_heuristic(asnode->prev, asnode->pos);
	} else {
		asnode->prev = pos;
		asnode->g = 0;
	}

	asnode->h = path_heuristic(pos, goal_pos);
	asnode->f = asnode->g + asnode->h;
	asnode->set = PATH_ASTAR_OPEN;
	
	return asnode;
}

int path_compare_asnodes(const void *a, const void *b) {
	return ((path_asnode_t *)a)->f - ((path_asnode_t *)b)->f;
}

// returns linked list of v3i
list_t *path_find(path_network_t *network, v3i start_pos, v3i goal_pos) {
	// check v3i's are both in the network, and in the same group
	int *startid, *goalid;

	startid = hashmap_get(network->ids, &start_pos);
	goalid = hashmap_get(network->ids, &goal_pos);

	if (startid == NULL || goalid == NULL || startid != goalid)
		return NULL;

	// A* algo
	heap_t *openset;
	hashmap_t *navigated;
	list_t *path;
	path_asnode_t *current, *neighbor;
	path_node_t *cur_node, *nbor_node;
	path_connect_t *nbor_connect;
	hashmap_iter_t *nbor_iter;
	double potential_g;
	int estimated_nodes, estimated_heap_depth;

	path = NULL;

	// estimate size of heap and nodes to prevent as many memcpy and realloc calls as possible
	estimated_nodes = v3i_magnitude(v3i_sub(start_pos, goal_pos));
	estimated_heap_depth = 1;

	while (estimated_nodes >> estimated_heap_depth)
		++estimated_heap_depth;

	openset = heap_create(estimated_heap_depth, path_compare_asnodes);
	navigated = hashmap_create(estimated_nodes, HASH_V3I);

	current = path_asnode_create(start_pos, NULL, goal_pos);
	heap_insert(openset, current);
	hashmap_set(navigated, &current->pos, current);

	while (openset->size > 0) {
		current = heap_pop(openset);

		// goal found, reconstruct path and return
		if (!v3i_compare(current->pos, goal_pos)) {
			path_asnode_t *trav = current;
			v3i *cur_pos;

			path = list_create();

			while (v3i_compare(trav->pos, start_pos)) {
				cur_pos = malloc(sizeof(v3i));
				*cur_pos = trav->pos;

				list_push(path, cur_pos);
				trav = hashmap_get(navigated, &trav->prev);
			}

			break;
		}

		current->set = PATH_ASTAR_CLOSED;
		cur_node = hashmap_get(network->nodes, &current->pos);

		// check node neighbors
		nbor_iter = hashmap_iter_create(cur_node->connects);

		while ((nbor_connect = hashmap_iter_next(nbor_iter)) != NULL) {
			nbor_node = ((path_connect_t *)nbor_connect)->node;
			neighbor = hashmap_get(navigated, &nbor_node->pos);

			if (neighbor == NULL) {
				neighbor = path_asnode_create(nbor_node->pos, current, goal_pos);
				hashmap_set(navigated, &neighbor->pos, neighbor);
				heap_insert(openset, neighbor);
			} else {
				potential_g = current->g + path_heuristic(neighbor->pos, current->pos);

				if (neighbor->set == PATH_ASTAR_CLOSED)
					if (potential_g >= neighbor->g)
						continue;

				if (neighbor->set != PATH_ASTAR_OPEN || potential_g < neighbor->g) {
					neighbor->prev = current->pos;
					neighbor->g = current->g + path_heuristic(neighbor->pos, neighbor->prev);
					neighbor->f = neighbor->g + neighbor->h;

					if (neighbor->set != PATH_ASTAR_OPEN) {
						neighbor->set = PATH_ASTAR_OPEN;
						heap_insert(openset, neighbor);
					}
				}
			}
		}

		free(nbor_iter);
	}

	hashmap_destroy(navigated, true);
	heap_destroy(openset, false);

	return path;
}



