#ifndef _APP_H_
#define _APP_H_

#include "halBoard.h"
#include "halUart.h"
#include "nwk.h"
#include "nwkDataReq.h"
#include "nwkRx.h"
#include "phy.h"
#include "sysTimer.h"
#include <avr/io.h>
#include <stdint.h>
#include <string.h>

typedef enum AppState_t {
	INIT,
	IDLE,
} AppState_t;

typedef enum AppMsgType_t {
	NACK = 0, // Negative Acknowledgement
	ACK = 1,  // Acknowledgement
	DISCOVER = 2, // c -> s  | Discover router
	OFFER = 3,    // s -> c  | Offer address config
	DECLINE = 4,  // c -> s  | Decline address config (prompts another OFFER)
	REQUEST = 5,  // c -> s  | Request address config
	RELEASE = 6,  // s -> c  | Release address config
	INFORM = 7,   // c -> s  | Propmts server to send config without initiation
	FULL = 8,     // s -> c  | Address space is full, no more clients can be added
	DATA= 9,
} AppMsgType_t;

typedef struct AppMsg_t {
	// header
	uint16_t dstAddr;
	uint16_t srcAddr;
	uint8_t size;
	AppMsgType_t msgType;
	// data
	uint8_t *data;
} AppMsg_t;


void APP_init(void);
void APP_timerHandler(SYS_Timer_t *timer);
bool APP_dataRecv(NWK_DataInd_t *ind);
void APP_dataOut(void);
void APP_dataConf(NWK_DataReq_t *req);
void APP_msgCreate(void);

#endif