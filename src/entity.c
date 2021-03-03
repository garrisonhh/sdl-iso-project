#include "entity.h"
#include "vector.h"
#include "collision.h"

void entity_destroy(entity_t *entity) {
	free(entity);
	entity = NULL;
}

void entity_tick(entity_t *entity, int ms, bbox_t *boxes, int num_boxes) {
	ray_t scaled_ray;
	bbox_t current_box;
	v3d resolved_dir, box_offset;
	double time;
	int i, axis;

	time = (double)ms / 1000;
	scaled_ray = entity->ray;
	scaled_ray.dir = v3d_scale(scaled_ray.dir, time);
	resolved_dir = (v3d){0, 0, 0};
	box_offset = v3d_scale(entity->size, -.5);

	sort_bboxes_by_vector_polarity(boxes, num_boxes, scaled_ray.dir);

	// resolve collisions by altering entity->ray.dir
	for (i = 0; i < num_boxes; i++) {
		current_box = boxes[i];
		current_box.pos = v3d_add(current_box.pos, box_offset);
		current_box.size = v3d_add(current_box.size, entity->size);

		ray_bbox_intersection(NULL, &resolved_dir, &axis, scaled_ray, current_box);
		if (axis >= 0) {
			scaled_ray.dir = resolved_dir;

			// kill inertia on collided axes
			v3d_set(&entity->ray.dir, axis, 0);
		}
	}

	entity->ray.pos = v3d_add(entity->ray.pos, scaled_ray.dir);
}
