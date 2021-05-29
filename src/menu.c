#include "menu.h"
#include "render.h"
#include "render/fonts.h"

menu_button_t *menu_button_create(SDL_Texture *texture, v2i pos, void (*func)(void)) {
	menu_button_t *button = malloc(sizeof(menu_button_t));

	button->texture = texture;
	button->func = func;
	
	button->rect.x = pos.x;
	button->rect.y = pos.y;

	SDL_QueryTexture(button->texture, NULL, NULL, &button->rect.w, &button->rect.h);

	return button;
}

menu_t *menu_create() {
	menu_t *screen = malloc(sizeof(menu_t));

	screen->buttons = array_create(0);
	screen->mouse_pt = (SDL_Point){-1, -1};
	screen->mouse_held = false;

	return screen;
}

void menu_destroy(menu_t *screen) {
	array_destroy(screen->buttons, true);
	free(screen);
}

void menu_add_button(menu_t *screen, SDL_Texture *texture, v2i pos, void (*func)()) {
	array_push(screen->buttons, menu_button_create(texture, pos, func));
}

void menu_add_text_button(menu_t *screen, const char *text, v2i pos, void (*func)()) {
	menu_add_button(screen, font_render_static(FONT_MENU, text), pos, func);
}

void menu_click(menu_t *screen, v2i pos) {
	SDL_Point point = {pos.x, pos.y};
	menu_button_t *button;

	for (size_t i = 0; i < screen->buttons->size; ++i) {
		button = screen->buttons->items[i];

		if (button->func != NULL && SDL_PointInRect(&point, &button->rect)) {
			button->func();
			break;
		}
	}
}

void menu_tick(menu_t *screen) {
	uint32_t mouse_state = SDL_GetMouseState(&screen->mouse_pt.x, &screen->mouse_pt.y);
	screen->mouse_held = mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT);
}

void menu_render(menu_t *screen) {
	menu_button_t *button;


	for (size_t i = 0; i < screen->buttons->size; ++i) {
		button = screen->buttons->items[i];
		
		if (button->func != NULL && SDL_PointInRect(&screen->mouse_pt, &button->rect)) {
			if (screen->mouse_held) {
				SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
				SDL_RenderFillRect(renderer, &button->rect);
			}

			SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
			SDL_RenderDrawRect(renderer, &button->rect);
		}

		SDL_RenderCopy(renderer, button->texture, NULL, &button->rect);
	}
}
