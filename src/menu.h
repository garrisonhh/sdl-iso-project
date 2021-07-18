#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>
#include <ghh/array.h>
#include "lib/vector.h"

enum menu_element_type_e {
	ELEM_LABEL,
	ELEM_BUTTON,
	ELEM_DYN_TEXT,
};
typedef enum menu_element_type_e menu_element_type_e;

typedef struct menu_element_data {
	menu_element_type_e type;

	SDL_Texture *texture;
	SDL_Rect rect;
} menu_element_data_t;

typedef struct menu_label {
	menu_element_data_t _data;
} menu_label_t;

typedef struct menu_button {
	menu_element_data_t _data;

	void (*func)(void);
} menu_button_t;

typedef struct menu_dynamicext {
	menu_element_data_t _data;

	char **text;
	char *cur_text;
} menu_dynamic_text_t;

union menu_element_t {
	menu_element_data_t data;
	menu_label_t label;
	menu_button_t button;
	menu_dynamic_text_t dyn_text;
};
typedef union menu_element_t menu_element_t;

typedef struct menu {
	SDL_Point mouse_pt;
	bool mouse_held;
	array_t *elements;
} menu_t;

menu_t *menu_create();
void menu_destroy(menu_t *);

void menu_add_label(menu_t *, SDL_Texture *, v2i pos);
void menu_add_text_label(menu_t *, const char *text, v2i pos);
void menu_add_button(menu_t *, SDL_Texture *, v2i pos, void (*func)());
void menu_add_text_button(menu_t *, const char *text, v2i pos, void (*func)());
void menu_add_dynamic_text(menu_t *, char **text, v2i pos);

void menu_click(menu_t *, v2i pos);

void menu_tick(menu_t *);
void menu_render(menu_t *);

#endif
