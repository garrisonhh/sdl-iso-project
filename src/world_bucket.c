#include "world.h"
#include "camera.h"

double world_bucket_y(entity_t *entity) {
	return (entity->ray.pos.x * camera.rndr_inc.x) + (entity->ray.pos.y * camera.rndr_inc.y);
}

int world_bucket_compare(const void *a, const void *b) {
	return (((world_bucket_y(*(entity_t **)a) - world_bucket_y(*(entity_t **)b)) >= 0) ? 1 : -1);
}

void world_bucket_add(world_t *world, v3i loc, entity_t *entity) {
	unsigned chunk_index, block_index;

	if (!world_indices(world, loc, &chunk_index, &block_index)) {
		printf("adding to out of bounds entity bucket.\n");
		exit(1);
	}

	chunk_t *chunk = world->chunks[chunk_index];

	if (chunk == NULL) {
		chunk = chunk_create();
		world->chunks[chunk_index] = chunk;
	}

	if (chunk->buckets[block_index] == NULL) {
		chunk->buckets[block_index] = list_create();
		list_push(world->buckets, chunk->buckets[block_index]);
	}
	
	list_push(chunk->buckets[block_index], entity);
	chunk->num_entities++;
}

void world_bucket_remove(world_t *world, v3i loc, entity_t *entity) {
	unsigned chunk_index, block_index;
	
	if (!world_indices(world, loc, &chunk_index, &block_index)) {
		printf("removing from out of bounds entity bucket.\n");
		exit(1);
	}

	chunk_t *chunk = world->chunks[chunk_index]; // will never be null

	list_remove(chunk->buckets[block_index], entity);
	chunk->num_entities--;
	
	if (chunk->buckets[block_index]->size == 0) {
		list_remove(world->buckets, chunk->buckets[block_index]);
		list_destroy(chunk->buckets[block_index], false);
		chunk->buckets[block_index] = NULL;
	}

	world_check_chunk(world, chunk_index);
}

void world_bucket_z_sort(list_t *bucket) {
	if (bucket->size == 2) {
		if (world_bucket_compare(&bucket->root->item, &bucket->tip->item) > 0)
			list_append(bucket, list_pop(bucket));
	} else if (bucket->size > 2) {
		int bucket_size = bucket->size, i = 0;
		entity_t **entities = malloc(sizeof(entity_t *) * bucket_size);

		while (bucket->size)
			entities[i++] = list_pop(bucket);

		qsort(entities, bucket_size, sizeof(entity_t *), world_bucket_compare);

		for (i = 0; i < bucket_size; ++i)
			list_append(bucket, entities[i]);

		free(entities);
	}
}
