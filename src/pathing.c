#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "pathing.h"
#include "vector.h"
#include "data_structures/hashmap.h"
#include "data_structures/hash_functions.h"
#include "data_structures/list.h"
#include "world.h"
#include "block.h"
#include "utils.h"

const double SQRT_2 = sqrt(2);

path_node_t *path_node_create(v3i pos) {
	path_node_t *node = (path_node_t *)malloc(sizeof(path_node_t));

	node->pos = pos;
	node->connects = hashmap_create(12, 1, hash_v3i);

	return node;
}

void path_node_destroy(path_node_t *node) {
	hashmap_destroy(node->connects, true);

	free(node);
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
		v3i_set(&delta, i, abs(v3i_get(&delta, i)));

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
		connect = (path_connect_t *)malloc(sizeof(path_connect_t));

		connect->weight = weight;
		connect->node = nodes[!i];

		hashmap_set(nodes[i]->connects, &nodes[!i]->pos, sizeof(v3i), connect);
	}
}

// TODO blocks precompute these
// TODO custom block path definitions/weights

bool path_block_empty(world_t *world, v3i loc) {
	block_t *block = block_get(world, loc);
	
	return block == NULL || block->coll_data->coll_type == BLOCK_COLL_NONE;
}

bool path_block_solid(world_t *world, v3i loc) {
	block_t *block = block_get(world, loc);
	
	return block != NULL && block->coll_data->coll_type == BLOCK_COLL_DEFAULT_BOX;
}

bool path_block_pathable(world_t *world, v3i loc) {
	// holy crap its THE COMMA OPERATOR IN ALL OF ITS GLORY
	return path_block_empty(world, loc) && (loc.z--, path_block_solid(world, loc));
}

// **will remove all buckets from nodes**
path_network_t *path_network_from_nodes(hashmap_t *nodes) {
	hashbucket_t *trav;
	list_t *active;
	path_network_t *network;
	path_node_t *node, *neighbor;
	void **connects;
	int id = 0, i;
	int *id_val;

	active = list_create();

	network = (path_network_t *)malloc(sizeof(path_network_t));
	network->nodes = hashmap_create(nodes->max_size, true, hash_v3i);
	network->ids = hashmap_create(nodes->max_size, true, hash_v3i);

	while (nodes->size > 0) {
		// get starting node
		i = 0;
		trav = nodes->buckets[i];

		while (trav == NULL) 
			trav = nodes->buckets[++i];

		// find all nodes connected to starting node, remove them from original nodes list
		// and add them to the network
		node = trav->value;

		// how to free this properly..?
		id_val = (int *)malloc(sizeof(int));
		*id_val = id;

		hashmap_remove(nodes, &node->pos, sizeof node->pos);
		hashmap_set(network->nodes, &node->pos, sizeof node->pos, node);
		hashmap_set(network->ids, &node->pos, sizeof node->pos, id_val);

		list_push(active, node);

		while (active->size > 0) {
			node = (path_node_t *)list_pop(active);

			if (node->connects->size) {
				connects = hashmap_values(node->connects);

				for (i = 0; i < node->connects->size; i++) {
					neighbor = ((path_connect_t *)connects[i])->node;

					if (hashmap_get(nodes, &neighbor->pos, sizeof neighbor->pos) != NULL) {
						hashmap_remove(nodes, &neighbor->pos, sizeof neighbor->pos);
						hashmap_set(network->nodes, &neighbor->pos, sizeof neighbor->pos, neighbor);
						hashmap_set(network->ids, &neighbor->pos, sizeof neighbor->pos, id_val);

						list_push(active, neighbor);
					}
				}

				free(connects);
			}
		}

		printf("group %i; size %lu; nodes left %lu.\n", id, network->nodes->size, nodes->size);
		++id;
	}

	network->groups = id;

	return network;
}

// TODO skip chunks with zero blocks in them?
// optimizing this is a weak todo, this function is not performance-critical by any means
path_network_t *path_generate_world_network(world_t *world) {
	hashmap_t *all_nodes;
	path_node_t *current, *neighbor;
	path_network_t *network;
	v3i pos, offset, nbor_pos, other1, other2;
	int i, temp;

	all_nodes = hashmap_create(world->block_size * world->block_size * 2, true, hash_v3i);

	FOR_CUBE(pos.x, pos.y, pos.z, 0, world->block_size) {
		if (path_block_pathable(world, pos)) {
			current = path_node_create(pos);
			hashmap_set(all_nodes, &pos, sizeof pos, current);

			// find  neighbors in cardinal directions
			offset.x = 1;
			offset.y = 0;

			for (i = 0; i < 4; i++) {
				for (offset.z = 1; offset.z >= -1; offset.z--) {
					nbor_pos = v3i_add(pos, offset);

					if ((neighbor = hashmap_get(all_nodes, &nbor_pos, sizeof nbor_pos)) != NULL)
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

					if ((neighbor = hashmap_get(all_nodes, &nbor_pos, sizeof nbor_pos)) != NULL) {
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

	printf("finished node generation, %lu nodes created.\n", all_nodes->size);

	network = path_network_from_nodes(all_nodes);

	hashmap_destroy(all_nodes, false);

	printf("finished network generation, %i groups found.\n", network->groups);

	return network;
}
