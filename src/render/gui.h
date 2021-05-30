#ifndef GUI_H
#define GUI_H

#include "../lib/vector.h"

typedef struct world_t world_t;

struct gui_data_t {
	double fps;
	int packets;
};
typedef struct gui_data_t gui_data_t;

extern gui_data_t GUI_DATA;

void gui_init(void);
void gui_load(void);

void gui_tick(void);
void gui_render(void);

void gui_toggle_debug(void);

#endif
