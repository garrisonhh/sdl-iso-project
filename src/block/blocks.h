#ifndef BLOCKS_H
#define BLOCKS_H

#include <json-c/json.h>
#include <stdlib.h>
#include "block.h"
#include "collision.h"

#define BLOCK_DECL(name) const size_t name = blocks_get_id(#name)

extern const v3d BLOCK_CENTER;
extern block_coll_data_t WALL_COLL_DATA;

void blocks_load(void);
void blocks_destroy(void);
size_t blocks_get_id(char *);
const char *blocks_get_name(size_t);
block_t *blocks_get(size_t);

#endif
