#ifndef SPRITES_H
#define SPRITES_H

#include <SDL2/SDL.h>
#include "lib/vector.h"

// also modify textures_load when modifying this enum
// sprite types define the mapping of animation sheets; how they are ticked
enum sprite_type_e {
	SPRITE_STATIC,
	SPRITE_HUMAN_BODY,
	SPRITE_HUMAN_HANDS,
	SPRITE_HUMAN_TOOL,

	NUM_SPRITE_TYPES
};
typedef enum sprite_type_e sprite_type_e;

// texture types
// sprites require an animation_t for state as well, see animation.*
typedef struct sprite {
	sprite_type_e type;

	// each row is an animation
	SDL_Texture *sheet;
	v2i pos, size;

	// array of animation lengths (corresponding to row lengths on sheet)
	int *anim_lengths;
	int num_anims;
} sprite_t;

void sprites_load(void);
void sprites_destroy(void);

sprite_t *sprite_from_key(const char *);

#endif
