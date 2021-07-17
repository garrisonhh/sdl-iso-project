#include <json-c/json.h>
#include <stdio.h>
#include "sprites.h"
#include "content.h"
#include "textures.h"
#include "meta.h"
#include <ghh/hashmap.h>

sprite_t **SPRITES = NULL;
size_t NUM_SPRITES;
hashmap_t *SPRITE_MAP;

sprite_t *load_sprite(const char *path, json_object *obj, hashmap_t *sprite_type_map) {
	sprite_t *sprite = malloc(sizeof(sprite_t));

	sprite->sheet = load_sdl_texture(path);

	if (content_has_key(obj, "type")) {
		const char *sprite_type_name;
		sprite_type_e *sprite_type;

		sprite_type_name = content_get_string(obj, "type");
		sprite_type = hashmap_get(sprite_type_map, sprite_type_name);

		if (sprite_type == NULL) {
			printf("\"%s\" is not a valid sprite type.\n", sprite_type_name);
			exit(1);
		}

		sprite->type = *sprite_type;
	} else {
		sprite->type = SPRITE_STATIC;
	}

	if (content_has_key(obj, "size"))
		sprite->size = content_get_v2i(obj, "size");
	else
		SDL_QueryTexture(sprite->sheet, NULL, NULL, &sprite->size.x, &sprite->size.y);

	sprite->pos = (v2i){-(sprite->size.x >> 1), -sprite->size.y};

	if (content_has_key(obj, "offset"))
		sprite->pos = v2i_add(sprite->pos, content_get_v2i(obj, "offset"));

	if (content_has_key(obj, "anim-lengths")) {
		array_t *anim_len_objs = content_get_array(obj, "anim-lengths");

		sprite->num_anims = array_size(anim_len_objs);
		sprite->anim_lengths = malloc(sizeof(int) * sprite->num_anims);

		for (size_t i = 0; i < sprite->num_anims; ++i)
			sprite->anim_lengths[i] = json_object_get_int(array_get(anim_len_objs, i));

		array_destroy(anim_len_objs, false);
	} else {
		sprite->anim_lengths = malloc(sizeof(int));
		sprite->num_anims = 1;
		sprite->anim_lengths[0] = 1;
	}

	return sprite;
}

void sprites_load() {
	size_t i;

	// sprite type map
	char *sprite_type_strings[] = {
		"static",
		"human-body",
		"human-hands",
		"human-tool",
	};
	sprite_type_e *sprite_type;
	hashmap_t *sprite_type_map = hashmap_create(NUM_SPRITE_TYPES * 2, -1, false);

	for (i = 0; i < NUM_SPRITE_TYPES; ++i) {
		sprite_type = malloc(sizeof(sprite_type_e));
		*sprite_type = (sprite_type_e)i;
		hashmap_set(sprite_type_map, sprite_type_strings[i], sprite_type);
	}

	// file
	json_object *file_obj;
	array_t *sprite_objects;

	file_obj = content_load_file("textures.json");
	sprite_objects = content_get_array(file_obj, "sprites");

	// set up globals
	NUM_SPRITES = array_size(sprite_objects);
	SPRITES = malloc(sizeof(sprite_t *) * NUM_SPRITES);
	SPRITE_MAP = hashmap_create(NUM_SPRITES * 2, -1, true);

	// load sprites
	json_object *sprite_obj;
	const char *name;
	char path[PATH_LEN];
	size_t *sprite_id;

	for (i = 0; i < NUM_SPRITES; ++i) {
		sprite_obj = array_get(sprite_objects, i);

		// name + path
		name = content_get_string(sprite_obj, "name");
		sprintf(path, content_get_string(sprite_obj, "path"));

		// load
		SPRITES[i] = load_sprite(path, sprite_obj, sprite_type_map);

		// save to array and hashmap
		sprite_id = malloc(sizeof(size_t));
		*sprite_id = i;
		hashmap_set(SPRITE_MAP, name, sprite_id);
	}

	// clean
	array_destroy(sprite_objects, false);
	hashmap_destroy(sprite_type_map, true);
	content_close_file(file_obj);
}

void sprites_destroy() {
	// SDL_Textures are freed with SDL_DestroyRenderer call
	for (size_t i = 0; i < NUM_SPRITES; ++i) {
		free(SPRITES[i]->anim_lengths);
		free(SPRITES[i]);
	}

	hashmap_destroy(SPRITE_MAP, true);
	free(SPRITES);
}

sprite_t *sprite_from_key(const char *key) {
	size_t *value = hashmap_get(SPRITE_MAP, key);

	if (value == NULL) {
		printf("key not found in SPRITE_MAP: %s\n", key);
		exit(1);
	}

	return SPRITES[*value];
}
