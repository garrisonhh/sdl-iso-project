#ifndef APP_H
#define APP_H

enum app_state_e {
	APP_EXIT,
	APP_MENU,
	APP_GAME,
	APP_TESTING,
};
typedef enum app_state_e app_state_e;

extern app_state_e APP_STATE;

void app_run(void);

void app_menu_init(void);
void app_menu_quit(void);

#endif
