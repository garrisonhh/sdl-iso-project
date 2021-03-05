#ifndef MEDIA_H
#define MEDIA_H

#include <SDL2/SDL.h>

void media_init(void);
void media_quit(void);
SDL_Texture *load_sdl_texture(char *path);

#endif
