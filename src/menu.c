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

menu_screen_t *menu_screen_create() {
	menu_screen_t *screen = malloc(sizeof(menu_screen_t));

	screen->buttons = array_create(0);

	return screen;
}

void menu_screen_destroy(menu_screen_t *screen) {
	array_destroy(screen->buttons, true);
	free(screen);
}

void menu_screen_add_button(menu_screen_t *screen, SDL_Texture *texture, v2i pos, void (*func)()) {
	array_push(screen->buttons, menu_button_create(texture, pos, func));
}

void menu_screen_add_text_button(menu_screen_t *screen, const char *text, v2i pos, void (*func)()) {
	menu_screen_add_button(screen, font_render_static(FONT_MENU, text), pos, func);
}

void menu_screen_click(menu_screen_t *screen, v2i pos) {
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

void menu_screen_render(menu_screen_t *screen) {
	menu_button_t *button;

	for (size_t i = 0; i < screen->buttons->size; ++i) {
		button = screen->buttons->items[i];

		SDL_RenderCopy(renderer, button->texture, NULL, &button->rect);
	}
}
