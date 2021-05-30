#include <json-c/json.h>
#include <stdlib.h>
#include "blocks.h"
#include "plant.h"
#include "../content.h"
#include "../textures.h"
#include "../lib/hashmap.h"

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

hashmap_t *blocks_load_plants(array_t *plant_objects) {
	hashmap_t *plant_models;
	json_object *plant_obj;
	plant_t *plant;
	const char *name;

	plant_models = hashmap_create(4, HASH_STRING);

	for (size_t i = 0; i < plant_objects->size; ++i) {
		plant_obj = plant_objects->items[i];
		plant = malloc(sizeof(plant_t));

		name = content_get_string(plant_obj, "name");
		*plant = (plant_t){
			.growth = 0,
			.growth_rate = content_get_double(plant_obj, "growth-rate"),
			.fullgrown = content_get_int(plant_obj, "fullgrown")
		};

		hashmap_set(plant_models, name, plant);
	}

	return plant_models;
}

void blocks_load_block(json_object *block_obj, size_t index,
						  hashmap_t *coll_type_map,
						  hashmap_t *block_type_map,
						  hashmap_t **block_subtype_maps) {
	block_t *block;
	block_coll_e *coll_type;
	block_coll_data_t *coll_data;
	block_type_e *block_type;
	const char *name, *texture;
	const char *coll_type_name, *block_type_name, *block_subtype_name;
	size_t *block_id;

	block = malloc(sizeof(block_t));

	name = content_get_string(block_obj, "name");

	// texture
	if (content_has_key(block_obj, "texture"))
		texture = content_get_string(block_obj, "texture");
	else
		texture = name;

	block->texture = texture_from_key((char *)texture);

	// coll data
	coll_data = malloc(sizeof(block_coll_data_t));

	if (content_has_key(block_obj, "collision")) {
		coll_type_name = content_get_string(block_obj, "collision");
		coll_type = hashmap_get(coll_type_map, coll_type_name);
		coll_data->coll_type = *coll_type;
	} else {
		coll_data->coll_type = BLOCK_COLL_DEFAULT_BOX;
	}

	switch (coll_data->coll_type) {
	case BLOCK_COLL_NONE:
		coll_data->bbox = NULL;
		break;
	case BLOCK_COLL_CUSTOM_BOX:
		coll_data->bbox = malloc(sizeof(bbox_t));
		*coll_data->bbox = content_get_bbox(block_obj, "bbox");
		break;
	default:
		coll_data->bbox = &BLOCK_DEFAULT_BOX;
		break;
	}

	block->coll_data = coll_data;

	// block type
	if (content_has_key(block_obj, "type")) {
		block_type_name = content_get_string(block_obj, "type");
		block_type = hashmap_get(block_type_map, block_type_name);
		block->type = *block_type;
	} else {
		block->type = BLOCK_STATELESS;
	}

	// block subtype
	switch (block->type) {
	case BLOCK_PLANT:
		block_subtype_name = content_get_string(block_obj, "subtype");
		block->state.plant = *(plant_t *)hashmap_get(block_subtype_maps[BLOCK_PLANT], block_subtype_name);
		break;
	default:
		break;
	}

	// tex_state
	block->tex_state = texture_zeroed_state();

	if (block->texture->type == TEX_SHEET && content_has_key(block_obj, "sheet-cell"))
		block->tex_state.cell = content_get_v2i(block_obj, "sheet-cell");

	// save model
	BLOCKS[index] = block;
	BLOCK_COLL_DATA[index] = coll_data;

	block_id = malloc(sizeof(size_t));
	*block_id = index;
	hashmap_set(BLOCK_MAP, name, block_id);
}

void blocks_load() {
	int i;

	// construct coll_type hashmap
	char *coll_type_strings[] = {
		"none",
		"default",
		"custom",
	};
	block_coll_e *coll_type;
	hashmap_t *coll_type_map = hashmap_create(NUM_BLOCK_COLL_TYPES * 2, HASH_STRING);

	for (i = 0; i < NUM_BLOCK_COLL_TYPES; ++i) {
		coll_type = malloc(sizeof(block_coll_e));
		*coll_type = i;

		hashmap_set(coll_type_map, coll_type_strings[i], coll_type);
	}

	// construct block_type hashmap
	char *block_type_strings[] = {
		"stateless",
		"plant",
	};
	block_type_e *block_type;
	hashmap_t *block_type_map = hashmap_create(NUM_BLOCK_TYPES * 2, HASH_STRING);

	for (i = 0; i < NUM_BLOCK_TYPES; ++i) {
		block_type = malloc(sizeof(block_coll_e));
		*block_type = i;

		hashmap_set(block_type_map, block_type_strings[i], block_type);
	}

	// load json file
	json_object *file, *block_subtypes;
	array_t *block_objects;

	file = content_load_file("assets/blocks.json");
	block_objects = content_get_array(file, "blocks");
	block_subtypes = content_get_obj(file, "subtypes");

	// load subtypes (indexed to block_type_e values)
	hashmap_t *block_subtype_maps[NUM_BLOCK_TYPES];
	array_t *subtype_arr;

	block_subtype_maps[BLOCK_STATELESS] = NULL;

	subtype_arr = content_get_array(block_subtypes, "plant");
	block_subtype_maps[BLOCK_PLANT] = blocks_load_plants(subtype_arr);
	array_destroy(subtype_arr, false);

	// set up globals
	NUM_BLOCKS = block_objects->size;
	BLOCKS = malloc(sizeof(block_t *) * NUM_BLOCKS);
	BLOCK_COLL_DATA = malloc(sizeof(block_coll_data_t *) * NUM_BLOCKS);
	BLOCK_MAP = hashmap_create(NUM_BLOCKS * 2, HASH_STRING);

	// load blocks
	for (i = 0; i < block_objects->size; ++i)
		blocks_load_block(block_objects->items[i], i, coll_type_map, block_type_map, block_subtype_maps);

	// clean up and exit
	hashmap_destroy(block_subtype_maps[BLOCK_PLANT], true);

	array_destroy(block_objects, false);
	hashmap_destroy(coll_type_map, true);
	hashmap_destroy(block_type_map, true);
	content_close_file(file);

	WALL_COLL_DATA = (block_coll_data_t){
		.coll_type = BLOCK_COLL_DEFAULT_BOX,
		.bbox = &BLOCK_DEFAULT_BOX,
	};
}

void blocks_destroy() {
	for (size_t i = 0; i < NUM_BLOCKS; i++) {
		if (BLOCK_COLL_DATA[i]->bbox != NULL && BLOCK_COLL_DATA[i]->bbox != &BLOCK_DEFAULT_BOX)
			free(BLOCK_COLL_DATA[i]->bbox);

		free(BLOCK_COLL_DATA[i]);
		block_destroy(BLOCKS[i]);
	}

	free(BLOCKS);
	BLOCKS = NULL;
	hashmap_destroy(BLOCK_MAP, true);
	BLOCK_MAP = NULL;
}

size_t blocks_get_id(char *key) {
	size_t *value;

	if ((value = hashmap_get(BLOCK_MAP, key)) == NULL) {
		printf("key not found in BLOCK_MAP: %s\n", key);
		exit(1);
	}

	return *value;
}

block_t *blocks_get(size_t id) {
	return BLOCKS[id];
}
