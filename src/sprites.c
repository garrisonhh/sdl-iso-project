#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <json-c/json.h>
#include "vector.h"
#include "render.h"
#include "media.h"
#include "sprites.h"
#include "hash.h"

sprite_t **sprites = NULL;
size_t num_sprites;
hash_table *sprite_table;

void sprites_load(json_object *file_obj) {
	json_object *sprite_arr_obj;
	sprite_arr_obj = json_object_object_get(file_obj, "sprites");
	num_sprites = json_object_array_length(sprite_arr_obj);

	json_object *current_png;
	const char *name;
	size_t *arr_index;

	sprites = (sprite_t **)calloc(num_sprites, sizeof(sprite_t *));
	sprite_table = hash_table_create(num_sprites * 1.3 + 1);

	for (size_t i = 0; i < num_sprites; i++) {
		char path[50];

		current_png = json_object_array_get_idx(sprite_arr_obj, i);
		name = json_object_get_string(json_object_object_get(current_png, "name"));
		sprintf(path, "assets/%s.png", name);

		sprites[i] = (sprite_t *)malloc(sizeof(sprite_t));
		sprites[i]->texture = load_sdl_texture(path);
	
		SDL_QueryTexture(sprites[i]->texture, NULL, NULL, &(sprites[i]->size.x), &(sprites[i]->size.y));
		sprites[i]->pos = (v2i){-(sprites[i]->size.x >> 1), -sprites[i]->size.y};
		
		arr_index = (size_t *)malloc(sizeof(size_t));
		*arr_index = i;
		hash_set(sprite_table, name, arr_index);
	}
}

void sprites_destroy() {
	for (size_t i = 0; i < num_sprites; i++) {
		SDL_DestroyTexture(sprites[i]->texture);
		free(sprites[i]);
	}

	free(sprites);
	sprites = NULL;
	hash_table_deep_destroy(sprite_table);
	sprite_table = NULL;
}

size_t sprite_index(const char *key) {
	void *value;

	if ((value = hash_get(sprite_table, key)) != NULL)
		return *(size_t *)value;

	printf("key not found in sprite_table: %s\n", key);
	exit(1);
}

void render_sprite(sprite_t *sprite, v2i pos) {
	SDL_Rect draw_rect = {
		pos.x + sprite->pos.x,
		pos.y + sprite->pos.y,
		sprite->size.x,
		sprite->size.y
	};
	SDL_RenderCopy(renderer, sprite->texture, NULL, &draw_rect);
}
