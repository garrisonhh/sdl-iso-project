#include <stdbool.h>
#include "world.h"
#include "vector.h"
#include "camera.h"
#include "render.h"
#include "utils.h"

// requires camera_init
camera_t camera = {
	.pos = (v2i){0, 0},
	.viewport = (SDL_Rect){0, 0, 0, 0},
	.rotation = 0,
};

void camera_update(void);

// project functions are high FPS impact so if the code looks stupid/repetitive
// that is why lol

// used for camera position only
v2i project_v3d_absolute(v3d v) {
	return (v2i){
		((v.x - v.y) * VOXEL_WIDTH) / 2,
		(((v.x + v.y) * VOXEL_WIDTH) / 4) - (v.z * VOXEL_Z_HEIGHT)
	};
}

v2i project_v3i(v3i v) {
	return project_v3d(v3d_from_v3i(v));
}

v2i project_v3d(v3d v) {
	int swp;

	v = v3d_sub(v, camera.center);

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
	};

	return (v2i){
		(((v.x - v.y) * VOXEL_WIDTH) / 2) + camera.center_screen.x,
		(((v.x + v.y) * VOXEL_WIDTH) / 4) - (v.z * VOXEL_Z_HEIGHT) + camera.center_screen.y
	};
}

void camera_init() {
	camera.view_circle.radius = SCREEN_HEIGHT >> 2;
	camera_set_scale(2);

	camera_update();
}

void camera_update() {
	int i;
	int min_val, max_val;

	v3i center = v3i_from_v3d(camera.center);

	camera.inc_render = (v3i){1, 1, 1};

	switch (camera.rotation) {
		case 1:
			camera.inc_render.x = -1;
			break;
		case 2:
			camera.inc_render.x = -1;
			camera.inc_render.y = -1;
			break;
		case 3:
			camera.inc_render.y = -1;
			break;
	}

	for (i = 0; i < 3; ++i) {
		min_val = MAX(0, v3i_get(&center, i) - camera.render_dist);
		max_val = MIN(camera.block_size, v3i_get(&center, i) + camera.render_dist);

		if (v3i_get(&camera.inc_render, i) > 0) {
			v3i_set(&camera.min_render, i, min_val);
			v3i_set(&camera.max_render, i, max_val);
		} else { // values swapped
			v3i_set(&camera.min_render, i, max_val - 1);
			v3i_set(&camera.max_render, i, min_val - 1);
		}
	}
}

void camera_set_block_size(int block_size) {
	camera.block_size = block_size;
	
	camera_update();
}

void camera_set_center(v3d center) {
	camera.center = center;
	camera.pos = v2i_sub(project_v3d_absolute(center), camera.center_screen);

	camera_update();
}

void camera_set_scale(int scale) {
	camera.scale = CLAMP(scale, 1, 16);

	// when I add variable window sizes this constant will need to be a variable
	camera.render_dist = 64 / camera.scale;

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
