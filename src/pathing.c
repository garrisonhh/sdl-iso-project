#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "pathing.h"
#include "vector.h"
#include "data_structures/hashmap.h"
#include "data_structures/hash_functions.h"
#include "data_structures/list.h"
#include "data_structures/heap.h"
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
void path_network_destroy(path_network_t *network) {
	hashmap_destroy(network->nodes, true);
	hashmap_destroy(network->ids, false); // TODO this leaks the ids

	free(network);
}
*/

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

		printf("group %i, size %lu; %lu nodes remaining.\n", id, network->nodes->size, nodes->size);
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

path_asnode_t *path_asnode_create(v3i pos, path_asnode_t *prev, v3i goal_pos) {
	path_asnode_t *asnode = (path_asnode_t *)malloc(sizeof(path_asnode_t));

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

	startid = (int *)hashmap_get(network->ids, &start_pos, sizeof start_pos);
	goalid = (int *)hashmap_get(network->ids, &goal_pos, sizeof goal_pos);

	if (startid != NULL && goalid != NULL && startid == goalid)
		return NULL;

	// A* algo
	heap_t *openset;
	hashmap_t *navigated;
	list_t *path;
	path_asnode_t *current, *neighbor;
	path_node_t *cur_node, *nbor_node;
	void **neighbors;
	double potential_g;
	int i;

	path = NULL;

	// TODO decide initial heap allocation based on distance?
	openset = heap_create(4, path_compare_asnodes);
	navigated = hashmap_create(64, true, hash_v3i);

	current = path_asnode_create(start_pos, NULL, goal_pos);
	heap_insert(openset, current);
	hashmap_set(navigated, &current->pos, sizeof current->pos, current);

	while (openset->size > 0) {
		current = heap_pop(openset);

		// goal found, reconstruct path and return
		if (!v3i_compare(current->pos, goal_pos)) {
			path_asnode_t *trav = current;
			v3i *cur_pos;

			path = list_create();

			while (v3i_compare(trav->pos, start_pos)) {
				cur_pos = (v3i *)malloc(sizeof(v3i));
				*cur_pos = trav->pos;

				list_push(path, cur_pos);
				trav = (path_asnode_t *)hashmap_get(navigated, &trav->prev, sizeof trav->prev);
			}

			break;
		}

		current->set = PATH_ASTAR_CLOSED;
		cur_node = (path_node_t *)hashmap_get(network->nodes, &current->pos, sizeof current->pos);

		neighbors = hashmap_values(cur_node->connects);

		// check node neighbors
		for (i = 0; i < cur_node->connects->size; i++) {
			nbor_node = ((path_connect_t *)neighbors[i])->node;
			neighbor = (path_asnode_t *)hashmap_get(navigated, &nbor_node->pos, sizeof nbor_node->pos);

			if (neighbor == NULL) {
				neighbor = path_asnode_create(nbor_node->pos, current, goal_pos);
				hashmap_set(navigated, &neighbor->pos, sizeof neighbor->pos, neighbor);
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

		free(neighbors);
	}

	hashmap_destroy(navigated, true);
	heap_destroy(openset, false);

	return path;
}
/* 
function A*(start,goal)
	closedset := the empty set    // The set of nodes already evaluated.
	openset := {start}    // The set of tentative nodes to be evaluated, initially containing the start node
	came_from := the empty map    // The map of navigated nodes.

	g_score[start] := 0    // Cost from start along best known path.
	// Estimated total cost from start to goal through y.
	f_score[start] := g_score[start] + heuristic_cost_estimate(start, goal)

	while openset is not empty
		current := the node in openset having the lowest f_score[] value
		if current = goal
			return reconstruct_path(came_from, goal)

		remove current from openset
		add current to closedset
		for each neighbor in neighbor_nodes(current)
			tentative_g_score := g_score[current] + dist_between(current,neighbor)
			if neighbor in closedset
				if tentative_g_score >= g_score[neighbor]
					continue

			if neighbor not in openset or tentative_g_score < g_score[neighbor] 
				came_from[neighbor] := current
				g_score[neighbor] := tentative_g_score
				f_score[neighbor] := g_score[neighbor] + heuristic_cost_estimate(neighbor, goal)
				if neighbor not in openset
					add neighbor to openset

	return failure
*/
