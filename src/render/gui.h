#ifndef GUI_H
#define GUI_H

#include "../lib/vector.h"

typedef struct world world_t;

typedef struct gui_data {
	double fps, loop_fps, main_fps;
	int packets;
} gui_data_t;

extern gui_data_t GUI_DATA;

void gui_init(void);
void gui_load(void);

void gui_tick(void);
void gui_render(void);

void gui_toggle_debug(void);

#endif
