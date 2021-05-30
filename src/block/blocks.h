#ifndef BLOCKS_H
#define BLOCKS_H

#include <json-c/json.h>
#include <stdlib.h>
#include "block.h"
#include "collision.h"

extern block_coll_data_t WALL_COLL_DATA;

void blocks_load(void);
void blocks_destroy(void);
size_t blocks_get_id(char *);
block_t *blocks_get(size_t);

#endif
