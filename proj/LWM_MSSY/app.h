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

#define APP_HEADER_SIZE 2
#define DEBUG_PRINT

typedef enum AppState_t {
  INIT,
  IDLE,
} AppState_t;

typedef enum AppMsgType_t {
  APP_NACK, // Negative Acknowledgement
  APP_ACK,  // Acknowledgement
  // Client msgs
  DISCOVER,  // c -> s  | Discover router
  DECLINE,   // c -> s  | Decline address config (prompts another OFFER)
  REQUEST,   // c -> s  | Request address config
  APP_SLEEP, // c -> s | Announce sleep mode
  RECONNECT, // c -> s | Wake up
  APPDATA,   // c -> s | Send data
  // Server msgs
  RELEASE,      // s -> c  | Release address config
  OFFER,        // s -> c  | Offer address config
  WAKE_UP,      // s -> c  | Wake up node
  REQUEST_DATA, // s -> c | Request data
} AppMsgType_t;

typedef enum SensorType_t {
  WEIGHT_SENSOR,
  OTHER,
} SensorType_t;

typedef struct AppMsg_t {
  // header
  uint8_t size;
  AppMsgType_t msgType;
  uint8_t sleep;
  // data
  uint8_t *data;
} AppMsg_t;

#define ROUTE_TABLE_SIZE 256
typedef struct RouteTable_t {
  uint16_t addr;
  uint8_t endpoint;
  SensorType_t type;
  bool sleeping;
  bool in_use;
} RouteTable_t;

void APP_init(void);
void APP_timerHandler(SYS_Timer_t *timer);
bool APP_dataRecv(NWK_DataInd_t *ind);
void APP_dataOut(void);
void APP_dataConf(NWK_DataReq_t *req);
void APP_msgCreate(void);

#endif
