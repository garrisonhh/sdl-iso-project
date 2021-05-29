#include "app.h"
#include "game.h"

app_state_e APP_STATE = APP_MAIN;

void app_run() {
	while (APP_STATE) {
		switch (APP_STATE) {
			case APP_MAIN:
				APP_STATE = APP_GAME;
				break;
			case APP_GAME:
				game_main();
				break;
			case APP_EXIT:
				break;
		}
	}
}
