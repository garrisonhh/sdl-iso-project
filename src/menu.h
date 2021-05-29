#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>
#include "data_structures/array.h"
#include "vector.h"

struct menu_button_t {
	SDL_Texture *texture;
	SDL_Rect rect;
	void (*func)(void);
};
typedef struct menu_button_t menu_button_t;

struct menu_screen_t {
	array_t *buttons;
};
typedef struct menu_screen_t menu_screen_t;

menu_screen_t *menu_screen_create();
void menu_screen_destroy(menu_screen_t *);

void menu_screen_add_button(menu_screen_t *, SDL_Texture *, v2i pos, void (*func)());
void menu_screen_add_text_button(menu_screen_t *, const char *text, v2i pos, void (*func)());

void menu_screen_click(menu_screen_t *, v2i pos);

void menu_screen_render(menu_screen_t *);

#endif
