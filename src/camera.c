#include <stdbool.h>
#include "world.h"
#include "vector.h"
#include "camera.h"
#include "render.h"
#include "utils.h"

const int VOXEL_HALF_W = VOXEL_WIDTH >> 1;
const int VOXEL_4TH_W = VOXEL_WIDTH >> 2;

// requires camera_init
camera_t camera = {
	.viewport = (SDL_Rect){0, 0, 0, 0},
	.rotation = 0,
};

void camera_update(void);

// project functions are high FPS impact and very visually important, so if the
// code looks stupid/repetitive/weirldy formed that is why lol
v2i project_v3i(v3i v) {
	v = camera_rotated_v3i(v);

	return (v2i){
		((v.x - v.y) * VOXEL_HALF_W) - camera.center.x,
		((v.x + v.y) * VOXEL_4TH_W) - (v.z * VOXEL_Z_HEIGHT) - camera.center.y
	};
}

v2i project_v3d_absolute(v3d v) {
	v = camera_rotated_v3d(v);

	return (v2i){
		((v.x - v.y) * VOXEL_WIDTH) / 2.0,
		(((v.x + v.y) * VOXEL_WIDTH) / 4.0) - (v.z * VOXEL_Z_HEIGHT)
	};
}

v2i project_v3d(v3d v) {
	v2i iso = project_v3d_absolute(v);

	return (v2i){iso.x - camera.center.x, iso.y - camera.center.y};
}

// use to adjust vectors for camera rotation
v3d camera_rotated_v3d(v3d v) {
	double swp;

	switch (camera.rotation) {
		case 1:
			SWAP(v.x, v.y, swp);
			v.y = -v.y;
			break;
		case 2:
			v.x = -v.x;
			v.y = -v.y;
			break;
		case 3:
			SWAP(v.x, v.y, swp);
			v.x = -v.x;
			break;
	}

	return v;
}

v3d camera_reverse_rotated_v3d(v3d v) {
	double swp;

	switch (camera.rotation) {
		case 3:
			SWAP(v.x, v.y, swp);
			v.y = -v.y;
			break;
		case 2:
			v.x = -v.x;
			v.y = -v.y;
			break;
		case 1:
			SWAP(v.x, v.y, swp);
			v.x = -v.x;
			break;
	}

	return v;
}

v3i camera_rotated_v3i(v3i v) {
	int swp;

	switch (camera.rotation) {
		case 1:
			SWAP(v.x, v.y, swp);
			v.y = -v.y;
			break;
		case 2:
			v.x = -v.x;
			v.y = -v.y;
			break;
		case 3:
			SWAP(v.x, v.y, swp);
			v.x = -v.x;
			break;
	}

	return v;
}

v3i camera_reverse_rotated_v3i(v3i v) {
	int swp;

	switch (camera.rotation) {
		case 3:
			SWAP(v.x, v.y, swp);
			v.y = -v.y;
			break;
		case 2:
			v.x = -v.x;
			v.y = -v.y;
			break;
		case 1:
			SWAP(v.x, v.y, swp);
			v.x = -v.x;
			break;
	}

	return v;
}

void camera_init() {
	v3d pos = (v3d){0, 0, 0};

	camera.view_circle.radius = SCREEN_HEIGHT >> 2;
	camera_set_scale(2);
	camera_set_pos(pos);

	camera_update();
}

void camera_update() {
	int i;
	int min_val, max_val;

	v3i center = v3i_from_v3d(camera.pos);

	camera.rndr_inc = (v3i){1, 1, 1};

	switch (camera.rotation) {
		case 1:
			camera.rndr_inc.x = -1;
			break;
		case 2:
			camera.rndr_inc.x = -1;
			camera.rndr_inc.y = -1;
			break;
		case 3:
			camera.rndr_inc.y = -1;
			break;
	}

	for (i = 0; i < 3; ++i) {
		min_val = MAX(0, v3i_get(&center, i) - camera.rndr_dist);
		max_val = MIN(camera.block_size - 1, v3i_get(&center, i) + camera.rndr_dist);

		if (v3i_get(&camera.rndr_inc, i) > 0) {
			v3i_set(&camera.rndr_start, i, min_val);
			v3i_set(&camera.rndr_end, i, max_val);
		} else { // values swapped
			v3i_set(&camera.rndr_start, i, max_val);
			v3i_set(&camera.rndr_end, i, min_val);
		}
	}
}

void camera_set_block_size(int block_size) {
	camera.block_size = block_size;
	
	camera_update();
}

void camera_set_pos(v3d pos) {
	camera.pos = pos;
	camera.center = v2i_sub(project_v3d_absolute(camera.pos), camera.center_screen);

	camera_update();
}

void camera_set_scale(int scale) {
	camera.scale = CLAMP(scale, 1, 16);

	// when I add variable window sizes this constant will need to be a variable
	camera.rndr_dist = 64 / camera.scale;

	camera.viewport.w = SCREEN_WIDTH / camera.scale;
	camera.viewport.h = SCREEN_HEIGHT / camera.scale;

	camera.center_screen = (v2i){
		camera.viewport.w >> 1,
		camera.viewport.h >> 1,
	};

	camera.view_circle.loc = camera.center_screen;

	camera_update();
}

// used for controlling with mouse wheel
void camera_change_scale(bool increase) {
	camera_set_scale((increase ? camera.scale << 1 : camera.scale >> 1));
}

void camera_rotate(bool clockwise) {
	camera.rotation += (clockwise ? 1 : -1);
	camera.rotation = (camera.rotation + 4) % 4;

	camera_update();
}
