#include "app.h"
#include "halUart.h"
#include <stdint.h>

static AppState_t appState;
static SYS_Timer_t timer;

static bool dataReqBusy = false;
static NWK_DataReq_t dataReq;
static uint8_t dataReqBuffer[NWK_MAX_PAYLOAD_SIZE];

static uint8_t uartBuffer[NWK_MAX_PAYLOAD_SIZE];
static uint8_t uartBufferPtr = 0;

static AppMsg_t appMsg;
static uint8_t appMsgBuffer[NWK_MAX_PAYLOAD_SIZE - sizeof(AppMsg_t)];

bool APP_dataRecv(NWK_DataInd_t *ind) {
	AppMsg_t *recv;
	memcpy(recv, ind->data, ind->size);

	HAL_UartWriteByte(recv->dstAddr >> 8);
	HAL_UartWriteByte(recv->dstAddr);
	HAL_UartWriteByte(recv->srcAddr >> 8);
	HAL_UartWriteByte(recv->srcAddr);
	HAL_UartWriteByte(recv->msgType);
	HAL_UartWriteByte(recv->size);
	for (int i = 0; i < recv->size; i++) {
		HAL_UartWriteByte(recv->data[i]);
	}

	return true;
}

void APP_msgCreate(void) {
	if (dataReqBusy || 0 == uartBufferPtr)
	return;

	memcpy(appMsgBuffer, uartBuffer, uartBufferPtr);

	appMsg.size = uartBufferPtr;
	appMsg.msgType = 0; // @TODO enum
	appMsg.data = dataReqBuffer;
}

void APP_dataConf(NWK_DataReq_t *req) {
	dataReqBusy = false;
	(void)req;
}

void APP_dataOut(void) {
	APP_msgCreate();

	dataReq.dstAddr = 1 - APP_ADDR;
	dataReq.dstEndpoint = APP_ENDPOINT;
	dataReq.srcEndpoint = APP_ENDPOINT;
	dataReq.options = NWK_OPT_ENABLE_SECURITY;
	dataReq.data = appMsgBuffer;
	dataReq.size = uartBufferPtr;
	dataReq.confirm = APP_dataConf;
	NWK_DataReq(&dataReq);

	uartBufferPtr = 0;
	dataReqBusy = true;
}

void APP_timerHandler(SYS_Timer_t *timer) {
	char test[] = "SYSLOG: timer is being reset";
	for (int i = 0; strlen(test); i++) {
		HAL_UartWriteByte(test[i]);
	}
	(void)timer;
}

void APP_init(void) {
	NWK_SetAddr(APP_ADDR);
	NWK_SetPanId(APP_PANID);
	PHY_SetChannel(APP_CHANNEL);
	PHY_SetRxState(true);

	NWK_OpenEndpoint(APP_ENDPOINT, APP_dataRecv);

	HAL_BoardInit();

	timer.interval = APP_FLUSH_TIMER_INTERVAL;
	timer.mode = SYS_TIMER_INTERVAL_MODE;
	timer.handler = APP_timerHandler;
}