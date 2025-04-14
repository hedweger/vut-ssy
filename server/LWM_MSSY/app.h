#ifndef _APP_H_
#define _APP_H_

#include <avr/io.h>
#include <stdint.h>
#include <string.h>

#include "halBoard.h"
#include "halUart.h"
#include "nwk.h"
#include "nwkDataReq.h"
#include "nwkRx.h"
#include "phy.h"
#include "sysTimer.h"

#define ROUTE_TABLE_SIZE 1024  // 65536 ???

typedef enum AppState_t {
  INIT,
  IDLE,
} AppState_t;

typedef enum AppMsgType_t {
  NACK = 0,      // Negative Acknowledgement
  ACK = 1,       // Acknowledgement
  DISCOVER = 2,  // c -> s  | Discover router
  DECLINE = 3,   // c -> s  | Decline address config (prompts another OFFER)
  REQUEST = 4,   // c -> s  | Request address config
  INFORM = 5,    // c -> s  | Propmts server to send config without initiation
  RELEASE = 6,   // s -> c  | Release address config
  OFFER = 7,     // s -> c  | Offer address config
  DATA = 8,
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
