#include <SDL2/SDL.h>
#include "entity.h"
#include "entity_human.h"
#include "world.h"
#include "vector.h"
#include "camera.h"
#include "utils.h"

entity_t *PLAYER;
const uint8_t *KEYBOARD;
const v3d DOWN = {1.0, 1.0, 0.0};
const v3d RIGHT = {1.0, -1.0, 0.0};

void player_init(world_t *world) {
	v3d pos = (v3d){5.0, 5.0, 15.0};

	PLAYER = entity_human_create();
	KEYBOARD = SDL_GetKeyboardState(NULL);

	world_spawn(world, PLAYER, pos);
}

void player_tick() {
	v3d move = {0, 0, 0};
	bool up = KEYBOARD[SDL_SCANCODE_W];
	bool down = KEYBOARD[SDL_SCANCODE_S];
	bool left = KEYBOARD[SDL_SCANCODE_A];
	bool right = KEYBOARD[SDL_SCANCODE_D];
	bool jump = KEYBOARD[SDL_SCANCODE_SPACE];

	if (up) {
		if (!down)
			move = v3d_sub(move, DOWN);
	} else if (down) {
		move = v3d_add(move, DOWN);
	}

	if (left) {
		if (!right)
			move = v3d_sub(move, RIGHT);
	} else if (right) {
		move = v3d_add(move, RIGHT);
	}

	if (!d_close(v3d_magnitude(move), 0)) {
		move = camera_reverse_rotated_v3d(v3d_normalize(move));

		// TODO human walk speed
		move = v3d_scale(move, 4.0);
	}

	PLAYER->ray.dir.x = move.x;
	PLAYER->ray.dir.y = move.y;

	if (PLAYER->on_ground && jump)
		PLAYER->ray.dir.z += 7.0; // TODO human jump height
}

v3d player_get_pos() {
	return PLAYER->ray.pos;
}
