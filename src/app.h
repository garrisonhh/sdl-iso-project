#ifndef APP_H
#define APP_H

enum app_state_e {
	APP_EXIT,
	APP_MAIN_MENU,
	APP_GAME,
};
typedef enum app_state_e app_state_e;

extern app_state_e APP_STATE;

void app_run(void);

#endif
