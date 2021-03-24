#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <json-c/json.h>
#include "render.h"
#include "textures.h"
#include "sprites.h"

void media_init() {
	int img_flags = IMG_INIT_PNG;
	if (!(IMG_Init(img_flags) & img_flags)) {
		printf("SDL_image could not initialize:\n%s\n", IMG_GetError());
		exit(1);
	}

	json_object *file_obj = json_object_from_file("assets/assets.json");
	
	textures_init();
	textures_load(file_obj);
	sprites_load(file_obj);
	
	while (json_object_put(file_obj) != 1)
		free(file_obj);
}

void media_quit() {
	textures_destroy();
	sprites_destroy();
	IMG_Quit();
}
