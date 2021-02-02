#include <stdio.h>
#include <SDL2/SDL.h>
#include "vector.h"

int main() {
	vector3 test3 = {1, 2, 0};
	SDL_Point test2;

	vector3ToIsometric(&test2, &test3, 32, 32);
}
