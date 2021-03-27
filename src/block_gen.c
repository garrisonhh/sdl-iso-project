#include <json-c/json.h>
#include <stdlib.h>
#include "hash.h"
#include "block.h"
#include "textures.h"

/*
 * this is similar but not identical to textures.c; textures can be reused
 * whenever but blocks cant, so this holds blocks to be copied instead of
 * used directly from the array
 */

block_t **blocks = NULL;
block_coll_data_t **block_coll_data;
size_t num_blocks;
hash_table *block_table;

bbox_t BLOCK_DEFAULT_BOX = {
	(v3d){0, 0, 0},
	(v3d){1, 1, 1}
};

void block_gen_load(json_object *file_obj) {
	size_t i, num_coll_types;
	size_t *arr_index;
	json_object *block_arr_obj, *current_block, *obj;
	int j;

	const char *name, *texture;
	bbox_t *bbox;
	ray_t *plane;

	hash_table *coll_type_table;
	block_coll_type *coll_type_ptr;
	num_coll_types = 4;
	char *coll_type_strings[] = {
		"none",
		"default",
		"custom",
		"chopped"
	};

	coll_type_table = hash_table_create(num_coll_types * 1.3 + 1);
	block_arr_obj = json_object_object_get(file_obj, "blocks");
	num_blocks = json_object_array_length(block_arr_obj);
	blocks = (block_t **)calloc(num_blocks, sizeof(block_t *));
	block_coll_data = (block_coll_data_t **)calloc(num_blocks, sizeof(block_coll_data_t *));
	block_table = hash_table_create(num_blocks * 1.3 + 1);

	for (i = 0; i < num_coll_types; i++) {
		coll_type_ptr = (block_coll_type *)malloc(sizeof(block_coll_type));
		*coll_type_ptr = (block_coll_type)i;
		hash_set(coll_type_table, coll_type_strings[i], coll_type_ptr);
	}

	for (i = 0; i < num_blocks; i++) {
		blocks[i] = (block_t *)malloc(sizeof(block_t));
		block_coll_data[i] = (block_coll_data_t *)malloc(sizeof(block_coll_data_t));
	
		current_block = json_object_array_get_idx(block_arr_obj, i);

		// name
		obj = json_object_object_get(current_block, "name");
		name = json_object_get_string(obj);

		// texture
		if ((obj = json_object_object_get(current_block, "texture")) != NULL)
			texture = json_object_get_string(obj);
		else
			texture = name;

		blocks[i]->texture = texture_index((char *)texture);

		// coll type
		if ((obj = json_object_object_get(current_block, "collision")) != NULL) {
			coll_type_ptr = (block_coll_type *)hash_get(coll_type_table,
														(char *)json_object_get_string(obj));

			if (coll_type_ptr == NULL) {
				printf("unknown collision type for block \"%s\".\n", name);
				exit(1);
			}

			block_coll_data[i]->coll_type = *coll_type_ptr;
		} else {
			block_coll_data[i]->coll_type = BLOCK_COLL_DEFAULT_BOX;
		}

		block_coll_data[i]->bbox = NULL;
		block_coll_data[i]->plane = NULL;

		switch (block_coll_data[i]->coll_type) {
			case BLOCK_COLL_CUSTOM_BOX:;
				bbox = (bbox_t *)malloc(sizeof(bbox_t));

				if ((obj = json_object_object_get(current_block, "bbox")) == NULL) {
					printf("no bbox provided for custom box \"%s\".\n", name);
					exit(1);
				} else if (json_object_array_length(obj) != 6) {
					printf("bbox provided for custom box of \"%s\" does not contain 6 values.\n", name);
					exit(1);
				}

				for (j = 0; j < 3; j++) {
					v3d_set(&bbox->pos, j, json_object_get_double(json_object_array_get_idx(obj, j)));
					v3d_set(&bbox->size, j, json_object_get_double(json_object_array_get_idx(obj, j + 3)));
				}

				block_coll_data[i]->bbox = bbox;

				break;
			case BLOCK_COLL_CHOPPED_BOX:;
				plane = (ray_t *)malloc(sizeof(ray_t));

				if ((obj = json_object_object_get(current_block, "plane")) == NULL) {
					printf("no plane provided for chopped box \"%s\".\n", name);
					exit(1);
				} else if (json_object_array_length(obj) != 6) {
					printf("plane provided for chopped box of \"%s\" does not contain 6 values.\n", name);
					exit(1);
				}

				for (j = 0; j < 3; j++) {
					v3d_set(&plane->pos, j, json_object_get_double(json_object_array_get_idx(obj, j)));
					v3d_set(&plane->dir, j, json_object_get_double(json_object_array_get_idx(obj, j + 3)));
				}

				block_coll_data[i]->plane = plane;
			case BLOCK_COLL_DEFAULT_BOX:;
				block_coll_data[i]->bbox = &BLOCK_DEFAULT_BOX;

				break;
			default:
				break;
		}

		blocks[i]->coll_data = block_coll_data[i];
		
		// add to indexing hash table
		arr_index = (size_t *)malloc(sizeof(size_t));
		*arr_index = i;
		hash_set(block_table, (char *)name, arr_index);
	}

	hash_table_deep_destroy(coll_type_table);
}

void block_gen_destroy() {
	for (size_t i = 0; i < num_blocks; i++)
		block_destroy(blocks[i]);

	free(blocks);
	blocks = NULL;
	hash_table_deep_destroy(block_table);
	block_table = NULL;
}

size_t block_gen_get_id(char *key) {
	size_t *value;

	if ((value = (size_t *)hash_get(block_table, key)) == NULL) {
		printf("key not found in block_table: %s\n", key);
		exit(1);
	}

	return *value;
}

block_t *block_gen_get(size_t id) {
	return blocks[id];
}
