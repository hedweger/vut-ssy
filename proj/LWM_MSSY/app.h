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
#include "appMsg.h"

#define APP_HEADER_SIZE 2
#define DEBUG_PRINT


typedef enum AppState_t {
  INIT,
  IDLE,
} AppState_t;

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
