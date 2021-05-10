#include <stdbool.h>
#include "world.h"
#include "vector.h"
#include "camera.h"
#include "render.h"
#include "utils.h"

// most vars require initialization
camera_t camera = {
	.pos = (v2i){0, 0},
	.viewport = (SDL_Rect){0, 0, 0, 0},
	.rotation = 0,
};

v2i project_v3d_absolute(v3d v) {
	return (v2i){
		((v.x - v.y) * VOXEL_WIDTH) / 2,
		(((v.x + v.y) * VOXEL_WIDTH) / 4) - (v.z * VOXEL_Z_HEIGHT)
	};
}

v2i project_v3i(v3i v) {
	return (v2i){
		camera.pos.x + (((v.x - v.y) * VOXEL_WIDTH) >> 1),
		camera.pos.y + (((v.x + v.y) * VOXEL_WIDTH) >> 2) - (v.z * VOXEL_Z_HEIGHT)
	};
}
// this is repeated code, left untouched because of high FPS impact
v2i project_v3d(v3d v) {
	return (v2i){
		camera.pos.x + (((v.x - v.y) * VOXEL_WIDTH) / 2),
		camera.pos.y + (((v.x + v.y) * VOXEL_WIDTH) / 4) - (v.z * VOXEL_Z_HEIGHT)
	};
}

void camera_init() {
	camera.view_circle.radius = SCREEN_HEIGHT >> 2;
	camera_set_scale(2);
}

void camera_update(world_t *world) {
	camera.pos = v2i_sub(camera.center_screen, project_v3d_absolute(world->player->ray.pos));
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
}

// used for controlling with mouse wheel
void camera_change_scale(bool increase) {
	camera_set_scale((increase ? camera.scale << 1 : camera.scale >> 1));
}

