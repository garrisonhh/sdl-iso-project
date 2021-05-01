#include <stdlib.h>
#include "block_collision.h"
#include "vector.h"
#include "data_structures/array.h"

v3i COLL_SORT_POLARITY = {1, 1, 1};

int block_coll_compare(const void *a, const void *b) {
	v3i *this, *other;
	int i, diff;

	this = &(**(block_collidable_t **)a).loc;
	other = &(**(block_collidable_t **)b).loc;

	for (i = 0; i < 3; i++) {
		diff = v3i_get(this, i) - v3i_get(other, i);
		
		if (v3i_get(&COLL_SORT_POLARITY, i) < 0)
			diff = -diff;

		if (diff != 0)
			return diff;
	}

	return 1;
}

void block_coll_array_sort(array_t *array, v3d entity_dir) {
	COLL_SORT_POLARITY = polarity_of_v3d(entity_dir);
	array_qsort(array, block_coll_compare);
}

