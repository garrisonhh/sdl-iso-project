#ifndef CONTENT_H
#define CONTENT_H

#include <json-c/json.h>
#include <stdbool.h>
#include "lib/array.h"
#include "lib/vector.h"
#include "collision.h"

void content_asset_path(char *dest, const char *rel_path);

json_object *content_load_file(const char *);
void content_close_file(json_object *);

bool content_has_key(json_object *obj, const char *key);
json_object *content_get_obj(json_object *, const char *key);
// destroy array without destroying values when done
array_t *content_array_from_obj(json_object *);

bool content_get_bool(json_object *, const char *key);
int content_get_int(json_object *, const char *key);
double content_get_double(json_object *, const char *key);
const char *content_get_string(json_object *, const char *key);
array_t *content_get_array(json_object *, const char *key);
v2i content_get_v2i(json_object *, const char *key);
v3d content_get_v3d(json_object *, const char *key);
bbox_t content_get_bbox(json_object *, const char *key);

#endif
