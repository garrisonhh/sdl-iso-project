#include <json-c/json.h>
#include <stdlib.h>
#include "block.h"
#include "content.h"
#include "textures.h"
#include "data_structures/hashmap.h"
#include "data_structures/hash_functions.h"

/*
 * this is similar but not identical to textures.c; textures can be reused
 * whenever but blocks can't, so this holds blocks to be copied instead of
 * used directly from the array
 */

block_t **BLOCKS = NULL; // the models blocks are based on
block_coll_data_t **BLOCK_COLL_DATA = NULL; // used solely to free pointers
hashmap_t *BLOCK_MAP = NULL;
size_t NUM_BLOCKS;

bbox_t BLOCK_DEFAULT_BOX = {
	(v3d){0, 0, 0},
	(v3d){1, 1, 1}
};
block_coll_data_t WALL_COLL_DATA; // collision data for world borders

void block_gen_load() {
	int i;

	// construct coll_type hashmap
	const int num_coll_types = 3;
	char *coll_type_strings[] = {
		"none",
		"default",
		"custom",
		// chopped is not in current development scope
	};
	block_coll_e *coll_type;
	hashmap_t *coll_type_map = hashmap_create(num_coll_types * 2, false, hash_string);

	for (i = 0; i < num_coll_types; ++i) {
		coll_type = (block_coll_e *)malloc(sizeof(block_coll_e));
		*coll_type = i;

		hashmap_set(coll_type_map, coll_type_strings[i], strlen(coll_type_strings[i]), coll_type);
	}

	// json block list
	json_object *file;
	array_t *block_objects;

	file = content_load_file("assets/blocks.json");
	block_objects = content_get_array(file, "blocks");

	// set up globals
	NUM_BLOCKS = block_objects->size;
	BLOCKS = (block_t **)malloc(sizeof(block_t *) * NUM_BLOCKS);
	BLOCK_COLL_DATA = (block_coll_data_t **)malloc(sizeof(block_coll_data_t *) * NUM_BLOCKS);
	BLOCK_MAP = hashmap_create(NUM_BLOCKS * 2, true, hash_string);

	// load blocks
	json_object *block_obj;
	block_coll_data_t *coll_data;
	const char *name, *texture, *coll_type_name;
	size_t *block_id;

	for (i = 0; i < NUM_BLOCKS; ++i) {
		BLOCKS[i] = (block_t *)malloc(sizeof(block_t));

		block_obj = block_objects->items[i];

		name = content_get_string(block_obj, "name");

		// texture
		if (content_has_key(block_obj, "texture"))
			texture = content_get_string(block_obj, "texture");
		else
			texture = name;

		// coll data
		coll_data = (block_coll_data_t *)malloc(sizeof(block_coll_data_t));

		if (content_has_key(block_obj, "collision")) {
			coll_type_name = content_get_string(block_obj, "collision");
			coll_data->coll_type = *(block_coll_e *)hashmap_get(coll_type_map,
																(char *)coll_type_name,
																strlen(coll_type_name));
		} else {
			coll_data->coll_type = BLOCK_COLL_DEFAULT_BOX;
		}

		coll_data->bbox = NULL;
		coll_data->plane = NULL;

		switch (coll_data->coll_type) {
			case BLOCK_COLL_NONE:
				break;
			case BLOCK_COLL_DEFAULT_BOX:
				coll_data->bbox = &BLOCK_DEFAULT_BOX;

				break;
			case BLOCK_COLL_CUSTOM_BOX:
				coll_data->bbox = (bbox_t *)malloc(sizeof(bbox_t));
				*coll_data->bbox = content_get_bbox(block_obj, "bbox");

				break;
			case BLOCK_COLL_CHOPPED_BOX:
				exit(1);

				break;
		}

		BLOCK_COLL_DATA[i] = coll_data;

		// save model to array and hashmap
		BLOCKS[i]->texture = texture_ptr_from_key((char *)texture);
		BLOCKS[i]->tex_state = block_tex_state_from(BLOCKS[i]->texture->type);
		BLOCKS[i]->coll_data = coll_data;

		block_id = (size_t *)malloc(sizeof(size_t));
		*block_id = i;
		hashmap_set(BLOCK_MAP, (char *)name, strlen(name), block_id);
	}

	// clean up and exit
	array_destroy(block_objects, false);
	hashmap_destroy(coll_type_map, true);
	content_close_file(file);

	WALL_COLL_DATA = (block_coll_data_t){
		.coll_type = BLOCK_COLL_DEFAULT_BOX,
		.bbox = &BLOCK_DEFAULT_BOX,
		.plane = NULL
	};
}

void block_gen_destroy() {
	for (size_t i = 0; i < NUM_BLOCKS; i++) {
		if (BLOCK_COLL_DATA[i]->bbox != NULL && BLOCK_COLL_DATA[i]->bbox != &BLOCK_DEFAULT_BOX)
			free(BLOCK_COLL_DATA[i]->bbox);
		if (BLOCK_COLL_DATA[i]->plane != NULL)
			free(BLOCK_COLL_DATA[i]->plane);

		free(BLOCK_COLL_DATA[i]);
		block_destroy(BLOCKS[i]);
	}

	free(BLOCKS);
	BLOCKS = NULL;
	hashmap_destroy(BLOCK_MAP, true);
	BLOCK_MAP = NULL;
}

size_t block_gen_get_id(char *key) {
	size_t *value;

	if ((value = (size_t *)hashmap_get(BLOCK_MAP, key, strlen(key))) == NULL) {
		printf("key not found in BLOCK_MAP: %s\n", key);
		exit(1);
	}

	return *value;
}

block_t *block_gen_get(size_t id) {
	return BLOCKS[id];
}
