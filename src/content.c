#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <json-c/json.h>
#include "textures.h"

// TODO rename to "content" or something?

void media_init() {
	int img_flags = IMG_INIT_PNG;
	if (!(IMG_Init(img_flags) & img_flags)) {
		printf("SDL_image could not initialize:\n%s\n", IMG_GetError());
		exit(1);
	}

	json_object *file_obj = json_object_from_file("assets/content.json");
	
	textures_init();
	textures_load(file_obj);
	
	while (json_object_put(file_obj) != 1)
		free(file_obj);
}

void media_quit() {
	textures_destroy();
	IMG_Quit();
}
