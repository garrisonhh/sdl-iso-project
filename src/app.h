#ifndef APP_H
#define APP_H

enum app_state_e {
	APP_EXIT = 0,
	APP_MAIN = 1,
	APP_GAME = 2,
};
typedef enum app_state_e app_state_e;

extern app_state_e APP_STATE;

void app_run(void);

#endif
