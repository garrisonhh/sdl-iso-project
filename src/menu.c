#include <stdio.h>
#include <ghh/array.h>
#include "menu.h"
#include "render.h"
#include "render/fonts.h"

void menu_element_find_rect(menu_element_t *element) {
	SDL_QueryTexture(element->data.texture, NULL, NULL, &element->data.rect.w, &element->data.rect.h);
}

void menu_element_populate(menu_element_t *element, menu_element_type_e type, SDL_Texture *texture, v2i pos) {
	element->data.type = type;
	element->data.texture = texture;
	element->data.rect.x = pos.x;
	element->data.rect.y = pos.y;

	menu_element_find_rect(element);
}

menu_element_t *menu_label_create(SDL_Texture *texture, v2i pos) {
	menu_element_t *element = malloc(sizeof(menu_element_t));

	menu_element_populate(element, ELEM_LABEL, texture, pos);

	return element;
}

menu_element_t *menu_button_create(SDL_Texture *texture, v2i pos, void (*func)(void)) {
	menu_element_t *element = malloc(sizeof(menu_element_t));

	menu_element_populate(element, ELEM_BUTTON, texture, pos);

	element->button.func = func;

	return element;
}

menu_element_t *menu_dyn_text_create(char **text, v2i pos) {
	menu_element_t *element = malloc(sizeof(menu_element_t));

	// texture will be set on first tick if text pointer is not null
	menu_element_populate(element, ELEM_DYN_TEXT, NULL, pos);

	element->dyn_text.text = text;
	element->dyn_text.cur_text = NULL;

	return element;
}

menu_t *menu_create() {
	menu_t *screen = malloc(sizeof(menu_t));

	screen->elements = array_create(0);
	screen->mouse_pt = (SDL_Point){-1, -1};
	screen->mouse_held = false;

	return screen;
}

void menu_destroy(menu_t *screen) {
	array_destroy(screen->elements, true);
	free(screen);
}

void menu_add_label(menu_t *screen, SDL_Texture *texture, v2i pos) {
	array_push(screen->elements, menu_label_create(texture, pos));
}

void menu_add_text_label(menu_t *screen, const char *text, v2i pos) {
	menu_add_label(screen, font_render_static(FONT_MENU, text), pos);
}

void menu_add_button(menu_t *screen, SDL_Texture *texture, v2i pos, void (*func)()) {
	if (func == NULL) {
		printf("don't make menu_buttons without a function, use a menu_label.\n");
		exit(1);
	}

	array_push(screen->elements, menu_button_create(texture, pos, func));
}

void menu_add_text_button(menu_t *screen, const char *text, v2i pos, void (*func)()) {
	menu_add_button(screen, font_render_static(FONT_MENU, text), pos, func);
}

void menu_add_dynamic_text(menu_t *screen, char **text, v2i pos) {
	array_push(screen->elements, menu_dyn_text_create(text, pos));
}

void menu_click(menu_t *screen, v2i pos) {
	SDL_Point point = {pos.x, pos.y};
	menu_element_t *element;

	for (size_t i = 0; i < array_size(screen->elements); ++i) {
		element = array_get(screen->elements, i);

		if (element->data.type == ELEM_BUTTON && SDL_PointInRect(&point, &element->data.rect)) {
			element->button.func();
			break;
		}
	}
}

void menu_tick(menu_t *screen) {
	menu_element_t *element;
	uint32_t mouse_state;

	mouse_state = SDL_GetMouseState(&screen->mouse_pt.x, &screen->mouse_pt.y);
	screen->mouse_held = mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT);

	for (size_t i = 0; i < array_size(screen->elements); ++i) {
		element = array_get(screen->elements, i);

		if (element->data.type == ELEM_DYN_TEXT) {
			if (element->dyn_text.cur_text == NULL
			 || strcmp(*element->dyn_text.text, element->dyn_text.cur_text)) {
				element->data.texture = font_render_static(FONT_MENU, *element->dyn_text.text);

				menu_element_find_rect(element);
			}
		}
	}
}

void menu_render(menu_t *screen) {
	menu_element_t *element;

	for (size_t i = 0; i < array_size(screen->elements); ++i) {
		element = array_get(screen->elements, i);

		// button highlighting
		if (element->data.type == ELEM_BUTTON) {
			if (SDL_PointInRect(&screen->mouse_pt, &element->data.rect)) {
				if (screen->mouse_held) {
					SDL_SetRenderDrawColor(RENDERER, 0x00, 0x00, 0x00, 0x7F);
					SDL_RenderFillRect(RENDERER, &element->data.rect);
				}

				SDL_SetRenderDrawColor(RENDERER, 0xFF, 0xFF, 0xFF, 0x7F);
				SDL_RenderDrawRect(RENDERER, &element->data.rect);
			}
		}

		if (element->data.texture != NULL)
			SDL_RenderCopy(RENDERER, element->data.texture, NULL, &element->data.rect);
	}
}
