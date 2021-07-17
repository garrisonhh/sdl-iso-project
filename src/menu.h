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

struct menu_element_data_t {
	menu_element_type_e type;

	SDL_Texture *texture;
	SDL_Rect rect;
};
typedef struct menu_element_data_t menu_element_data_t;

struct menu_label_t {
	menu_element_data_t _data;
};
typedef struct menu_label_t menu_label_t;

struct menu_button_t {
	menu_element_data_t _data;

	void (*func)(void);
};
typedef struct menu_button_t menu_button_t;

struct menu_dynamic_text_t {
	menu_element_data_t _data;

	char **text;
	char *cur_text;
};
typedef struct menu_dynamic_text_t menu_dynamic_text_t;

union menu_element_t {
	menu_element_data_t data;
	menu_label_t label;
	menu_button_t button;
	menu_dynamic_text_t dyn_text;
};
typedef union menu_element_t menu_element_t;

struct menu_t {
	SDL_Point mouse_pt;
	bool mouse_held;
	array_t *elements;
};
typedef struct menu_t menu_t;

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
