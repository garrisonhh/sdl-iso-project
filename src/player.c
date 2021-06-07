#include <SDL2/SDL.h>
#include <math.h>
#include "world.h"
#include "camera.h"
#include "raycast.h"
#include "entity/entity.h"
#include "entity/human.h"
#include "block/blocks.h"
#include "lib/vector.h"
#include "lib/utils.h"

const double SIN_PI_6 = sin(M_PI / 6);

bool GODMODE = false;

entity_t *PLAYER;
const uint8_t *KEYBOARD;
const v3d DOWN = {1.0, 1.0, 0.0};
const v3d RIGHT = {1.0, -1.0, 0.0};

void player_init(world_t *world) {
	v3d pos = (v3d){5.0, 5.0, 15.0};

	PLAYER = (entity_t *)human_create();
	KEYBOARD = SDL_GetKeyboardState(NULL);

	world_spawn(world, PLAYER, pos);
}

void player_tick() {
	//v2i mouse_pos;
	//uint32_t mouse = SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);

	// movement
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
		move = v3d_scale(move, HUMAN_WALK_VELOCITY);
	}

	PLAYER->data.ray.dir.x = move.x;
	PLAYER->data.ray.dir.y = move.y;

	if (GODMODE) {
		if (jump)
			PLAYER->data.ray.dir.z = HUMAN_WALK_VELOCITY;
	} else {
		if (PLAYER->data.on_ground && jump)
			PLAYER->data.ray.dir.z += HUMAN_JUMP_VELOCITY;
	}

	// tool
	/*
	if (mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {
		if (d_close(move.x + move.y, 0)) {
			v2d pos;
			v2i dir;

			pos.x = mouse_pos.x - (SCREEN_WIDTH >> 1);
			pos.y = (mouse_pos.y - (SCREEN_HEIGHT >> 1)) * 2;
			pos = v2d_normalize(pos);

			for (int i = 0; i < 2; ++i) {
				if (v2d_IDX(pos, i) > SIN_PI_6)
					v2i_IDX(dir, i) = 1;
				else if (v2d_IDX(pos, i) < -SIN_PI_6)
					v2i_IDX(dir, i) = -1;
				else
					v2i_IDX(dir, i) = 0;
			}

			PLAYER->data.last_dir.x = dir.x;
			PLAYER->data.last_dir.y = dir.y;

			PLAYER->data.last_dir = camera_reverse_rotated_v3d(PLAYER->data.last_dir);
		}

		entity_human_use_tool(PLAYER);
	}
	*/
}

void player_click(world_t *world, v2i mouse_pos) {
	int axis;
	v3i loc;
	ray_t ray;

	ray = (ray_t){
		.pos = un_project(mouse_pos, (double)world->block_size - 0.5),
		.dir = camera.view_dir
	};

	if (raycast_to_block(world, ray, raycast_block_exists, &loc, &axis)) {
		BLOCK_DECL(stone);

		v3i_IDX(loc, axis) += (v3d_IDX(camera.view_dir, axis) < 0 ? 1 : -1);

		world_set(world, loc, stone);
	}
}

void player_toggle_godmode() {
	GODMODE = !GODMODE;
}

v3d player_get_pos() {
	return PLAYER->data.ray.pos;
}
