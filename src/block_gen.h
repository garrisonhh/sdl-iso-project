#ifndef BLOCK_GEN_H
#define BLOCK_GEN_H

#include <json-c/json.h>
#include <stdlib.h>
#include "block.h"

void block_gen_load(json_object *);
void block_gen_destroy(void);
size_t block_gen_get_id(char *);
block_t *block_gen_get(size_t);

#endif
