#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>
#include "lib/array.h"
#include "lib/vector.h"

struct menu_button_t {
	SDL_Texture *texture;
	SDL_Rect rect;
	void (*func)(void);
};
typedef struct menu_button_t menu_button_t;

struct menu_t {
	SDL_Point mouse_pt;
	bool mouse_held;
	array_t *buttons;
};
typedef struct menu_t menu_t;

menu_t *menu_create();
void menu_destroy(menu_t *);

void menu_add_button(menu_t *, SDL_Texture *, v2i pos, void (*func)());
void menu_add_text_button(menu_t *, const char *text, v2i pos, void (*func)());

void menu_click(menu_t *, v2i pos);

void menu_tick(menu_t *);
void menu_render(menu_t *);

#endif
