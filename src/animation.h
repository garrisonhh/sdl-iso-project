#ifndef ANIMATION_H
#define ANIMATION_H

#include "vector.h"

typedef struct entity_t entity_t;
typedef struct animation_t animation_t;

struct animation_t {
	v2i cell;
	double state;
};
typedef struct animation_t animation_t;

void anim_human_body(entity_t *, animation_t *);
void anim_human_hands(entity_t *, animation_t *);

#endif
