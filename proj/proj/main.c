#include "app.h"
#include "halUart.h"
#include "sys.h"

static AppState_t appState;

static void APP_TaskHandler(void) {
	switch (appState) {
		case INIT: {
			APP_init();
			appState = IDLE;
		} break;
		case IDLE:
		default:
		break;
	}
}

int main(void) {
	SYS_Init();
	HAL_UartInit(38400);
	HAL_UartWriteByte('a');

	while (1) {
		SYS_TaskHandler();
		HAL_UartTaskHandler();
		APP_TaskHandler();
	}
}