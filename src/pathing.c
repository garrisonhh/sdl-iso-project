#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "pathing.h"
#include "vector.h"
#include "data_structures/hashmap.h"
#include "data_structures/hash_functions.h"
#include "world.h"
#include "block.h"
#include "utils.h"

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

void path_node_connect(path_node_t *a, path_node_t *b) {
	path_node_t *nodes[2];
	path_connect_t *connect;
	double weight, term;
	int i;

	nodes[0] = a;
	nodes[1] = b;

	// weight is the same both ways for now
	for (i = 0; i < 3; i++) {
		term = v3i_get(&a->pos, i) - v3i_get(&b->pos, i);
		weight += term * term;
	}

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

bool path_block_pathable(world_t *world, v3i loc) {
	// holy crap its THE COMMA OPERATOR IN ALL OF ITS GLORY
	return path_block_empty(world, loc) && (loc.z--, !path_block_empty(world, loc));
}

/* 
 * 1) construct a hashmap of valid path nodes
 * 2) connect nodes
 * 3) determine unconnected groups of nodes using a fill algorithm and separate them
 * 4) store separated groups in some kind of data structure for future use
 *
 * currently not worrying about future ability to modify this structure, make it work first and
 * then iterate
 *
 * there are also a lot of optimizations to be done here
 * - skipping chunks with zero blocks in them
 * - removing path_connect_t, instead use a singular hashmap of {v3i : v3i}
 */
hashmap_t *path_map_construct(world_t *world) {
	hashmap_t *all_nodes;
	v3i pos, offset, nbor_pos;
	path_node_t *current, *neighbor;
	int i, temp;

	// generate full list of connected nodes
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
			/* TODO
			offset.x = 1;
			offset.y = 1;

			for (i = 0; i < 4; i++) {
				for (offset.z = 1; offset.z >= 0; offset.z--) {
				}
				
				SWAP(offset.x, offset.y, temp);
				offset.y = -offset.y;
			} */
		}
	}

	printf("finished node generation, %lu nodes created\n", all_nodes->size);

	int freq[16];

	for (i = 0; i < 16; i++)
		freq[i] = 0;

	path_node_t **nodes = (path_node_t **)hashmap_values(all_nodes);

	for (i = 0; i < all_nodes->size; i++)
		freq[nodes[i]->connects->size]++;

	printf("connection frequency: ");

	for (i = 0; i < 16; i++)
		printf("%6d", freq[i]);

	printf("\n");

	return all_nodes;
}
