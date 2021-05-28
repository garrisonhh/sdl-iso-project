#include <stdbool.h>
#include "camera.h"
#include "vector.h"
#include "utils.h"

const int VOXEL_HALF_W = VOXEL_WIDTH >> 1;
const int VOXEL_4TH_W = VOXEL_WIDTH >> 2;

v2i project_v3d_absolute(v3d);
void camera_set_rotation(int);

// requires camera_init
camera_t camera = {
	.viewport = (SDL_Rect){0, 0, 0, 0},
	.rotation = 0,
};

void camera_init() {
	v3d pos = (v3d){0, 0, 0};

	camera.view_circle.radius = SCREEN_HEIGHT >> 2;
	camera_set_scale(2);
	camera_set_pos(pos);
	camera_set_rotation(0);
}

void camera_set_block_size(int block_size) {
	camera.block_size = block_size;
}

void camera_set_pos(v3d pos) {
	camera.pos = pos;
	camera.center = v2i_sub(project_v3d_absolute(camera.pos), camera.center_screen);
}

void camera_set_scale(int scale) {
	camera.scale = CLAMP(scale, 1, 16);

	camera.viewport.w = SCREEN_WIDTH / camera.scale;
	camera.viewport.h = SCREEN_HEIGHT / camera.scale;

	camera.center_screen = (v2i){
		camera.viewport.w >> 1,
		camera.viewport.h >> 1,
	};

	camera.view_circle.loc = camera.center_screen;
	camera.view_circle.radius = camera.viewport.w >> 2;

	camera.rndr_dist = camera.viewport.w / VOXEL_WIDTH;
}

// used for controlling with mouse wheel
void camera_scale(bool increase) {
	camera_set_scale((increase ? camera.scale << 1 : camera.scale >> 1));
}

void camera_set_rotation(int rotation) {
	camera.rotation = rotation;
	camera.facing = (v3i){-1, -1, -1};

	switch (camera.rotation) {
		case 1:
			camera.facing.x = 1;
			break;
		case 2:
			camera.facing.x = 1;
			camera.facing.y = 1;
			break;
		case 3:
			camera.facing.y = 1;
			break;
	}
}

void camera_rotate(bool clockwise) {
	camera_set_rotation((camera.rotation + (clockwise ? 1 : -1) + 4) % 4);
}

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

