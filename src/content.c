#include <SDL2/SDL.h>
#include <json-c/json.h>
#include <stdio.h>
#include <stdbool.h>
#include "textures.h"
#include "collision.h"
#include "meta.h"
#include "lib/array.h"
#include "lib/vector.h"

void content_asset_path(char *dest, const char *rel_path) {
	char buffer[PATH_LEN];

	sprintf(buffer, BASE_DIRECTORY "/assets");

	if (rel_path[0] != '/')
		strcat(buffer, "/");

	strcat(buffer, rel_path);
	strcpy(dest, buffer);
}

json_object *content_load_file(const char *asset_path) {
	json_object *file_obj;
	char path[PATH_LEN];

	content_asset_path(path, asset_path);
	file_obj = json_object_from_file(path);

	if (file_obj == NULL) {
		printf("json file at \"%s\" could not be loaded.\n", path);
		exit(1);
	}

	return file_obj;
}

void content_close_file(json_object *file_obj) {	
	// this is an extremely mysterious function I do not understand
	// wrapped it in content_close_file() for consistency and so I dont get confused in future
	while (json_object_put(file_obj) != 1)
		;
}

bool content_has_key(json_object *obj, const char *key) {
	return json_object_object_get(obj, key) != NULL;
}

// the json-c "array_list" is basically the same thing as my implementation lol
// this just steals the data `\("/)/`
array_t *content_array_from_obj(json_object *obj) {
	array_t *arr;
	array_list *json_arr;

	json_arr = json_object_get_array(obj);
	arr = array_create(json_arr->length);

	memcpy(arr->items, json_arr->array, sizeof(void *) * json_arr->length);

	arr->size = json_arr->length;

	return arr;
}

v2i content_v2i_from_obj(json_object *obj) {
	array_t *arr = content_array_from_obj(obj);

	if (arr->size != 2) {
		printf("v2i could not be retrieved from json array with invalid length.\n");
		exit(1);
	}

	v2i v = {
		json_object_get_int(arr->items[0]),
		json_object_get_int(arr->items[1])
	};

	array_destroy(arr, false);

	return v;
}

v3d content_v3d_from_obj(json_object *obj) {
	array_t *arr = content_array_from_obj(obj);

	if (arr->size != 3) {
		printf("v3d could not be retrieved from json array with invalid length.\n");
		exit(1);
	}

	v3d v;

	for (int i = 0; i < 3; ++i)
		v3d_IDX(v, i) = json_object_get_double(arr->items[i]);

	array_destroy(arr, false);

	return v;
}

bbox_t content_bbox_from_obj(json_object *obj) {
	array_t *arr = content_array_from_obj(obj);

	if (arr->size != 2) {
		printf("bbox could not be retrieved from json array with invalid length.\n");
		exit(1);
	}

	bbox_t bbox = {
		content_v3d_from_obj((json_object *)arr->items[0]),
		content_v3d_from_obj((json_object *)arr->items[1]),
	};

	array_destroy(arr, false);

	return bbox;
}

json_object *content_get_obj(json_object *obj, const char *key) {
	json_object *retrieved = json_object_object_get(obj, key);

	if (retrieved == NULL) {
		printf("key \"%s\" could not be retrieved from a json object.\n", key);
		exit(1);
	}

	return retrieved;
}

bool content_get_bool(json_object *obj, const char *key) {
	return json_object_get_boolean(content_get_obj(obj, key));
}

int content_get_int(json_object *obj, const char *key) {
	return json_object_get_int(content_get_obj(obj, key));
}

double content_get_double(json_object *obj, const char *key) {
	return json_object_get_double(content_get_obj(obj, key));
}

const char *content_get_string(json_object *obj, const char *key) {
	return json_object_get_string(content_get_obj(obj, key));
}

array_t *content_get_array(json_object *obj, const char *key) {
	return content_array_from_obj(content_get_obj(obj, key));
}

v2i content_get_v2i(json_object *obj, const char *key) {
	return content_v2i_from_obj(content_get_obj(obj, key));
}

v3d content_get_v3d(json_object *obj, const char *key) {
	return content_v3d_from_obj(content_get_obj(obj, key));
}

bbox_t content_get_bbox(json_object *obj, const char *key) {
	return content_bbox_from_obj(content_get_obj(obj, key));
}
