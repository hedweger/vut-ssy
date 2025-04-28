#include <stdint.h>
#include <string.h>

#include "app.h"
#include "halUart.h"
#include "nwk.h"
#include "nwkDataReq.h"
#include "nwkRx.h"

static AppState_t appState;
static SYS_Timer_t timer;

static bool dataReqBusy = false;
static NWK_DataReq_t dataReq;
static uint8_t dataReqBuffer[NWK_MAX_PAYLOAD_SIZE];

static uint8_t uartBuffer[NWK_MAX_PAYLOAD_SIZE];
static uint8_t uartBufferPtr = 0;

static AppMsg_t appMsg;
static uint8_t *appMsgPointer = &appMsg;
static uint8_t appMsgBuffer[NWK_MAX_PAYLOAD_SIZE - sizeof(AppMsg_t)];

static uint16_t routeTable[ROUTE_TABLE_SIZE];
static uint16_t routeTablePtr = 0;

void HAL_UartBytesReceived(uint16_t bytes) { HAL_UartWriteString(bytes); }

void APP_dataRoute(NWK_DataInd_t *ind) {
  // if (dataReqBusy || 0 == uartBufferPtr) return;
  // just send the DATA msg back for now..
  dataReq.dstAddr = ind->srcAddr;
  dataReq.dstEndpoint = APP_ENDPOINT;
  dataReq.srcEndpoint = APP_ENDPOINT;
  dataReq.options = NWK_OPT_ENABLE_SECURITY;
  dataReq.data = ind->data;
  dataReq.size = ind->size;
  dataReq.confirm = APP_dataConf;
  NWK_DataReq(&dataReq);

  uartBufferPtr = 0;
  dataReqBusy = true;
}

void APP_dataSend(AppMsgType_t msgType, uint8_t addr) {
  appMsg.msgType = msgType;
  switch (msgType) {
    case OFFER:
      memcpy(appMsg.data, &routeTablePtr, sizeof(uint16_t));
      break;
    default:
      appMsg.data = NULL;
      break;
  }
  uint8_t app_size = sizeof(appMsg);
  memcpy(appMsgBuffer, &appMsg, app_size);

  dataReq.dstAddr = addr;
  dataReq.dstEndpoint = APP_ENDPOINT;
  dataReq.srcEndpoint = APP_ENDPOINT;
  dataReq.options = NWK_OPT_ENABLE_SECURITY;
  dataReq.size = app_size;
  dataReq.data = NULL;
  
  memcpy(dataReq.data, appMsgBuffer, app_size);
  
  dataReq.confirm = APP_dataConf;
  NWK_DataReq(&dataReq);

  uartBufferPtr = 0;
  dataReqBusy = true;
}

#if DESIGNATION == 1  // client
bool APP_dataRecv(NWK_DataInd_t *ind) {
  AppMsg_t *recv = (AppMsg_t *)ind->data;

  switch (recv->msgType) {
    case RELEASE:
      APP_dataSend(ACK, ind->srcAddr);
      break;
    case OFFER:
      APP_dataSend(ACK, ind->srcAddr);
      break;
    case DATA:
      for (uint8_t i = 0; i < sizeof(recv->data); i++) {
        HAL_UartWriteByte(recv->data[i]);
      }
      break;
  }
}
#else
bool APP_dataRecv(NWK_DataInd_t *ind) {
  AppMsg_t *recv = (AppMsg_t *)ind->data;

  switch (recv->msgType) {
    case DATA:
      APP_dataRoute(ind);
      break;
    case DISCOVER:
      routeTablePtr++;
      routeTable[routeTablePtr] = routeTablePtr;
      APP_dataSend(OFFER, ind->srcAddr);
      break;
    case DECLINE:
      APP_dataSend(OFFER, ind->srcAddr);
      break;
    case REQUEST:
      APP_dataSend(ACK, ind->srcAddr);
      break;
    case INFORM:
      APP_dataSend(ACK, ind->srcAddr);
      break;
    default:
      HAL_UartWriteString("Unknown msg type: ");
      HAL_UartWriteByte(recv->msgType);
      break;
  }

#ifdef DEBUG_PRINT
  HAL_UartWriteByte('\n');
  HAL_UartWriteString("To address: ");
  HAL_UartWriteByte(ind->dstAddr >> 8);
  HAL_UartWriteByte(ind->dstAddr);
  HAL_UartWriteByte('\n');
  HAL_UartWriteString("From address: ");
  HAL_UartWriteByte(ind->srcAddr >> 8);
  HAL_UartWriteByte(ind->srcAddr);
  HAL_UartWriteByte('\n');
  HAL_UartWriteString("Message type: ");
  HAL_UartWriteByte(recv->msgType);
  HAL_UartWriteByte('\n');
  HAL_UartWriteString("Size: ");
  HAL_UartWriteByte(ind->size);
  HAL_UartWriteByte('\n');
  HAL_UartWriteString("Payload: ");
  for (int i = 0; i < ind->size; i++) {
    HAL_UartWriteByte(recv->data[i]);
  }
  HAL_UartWriteByte('\n');
#endif

  return true;
}
#endif

void APP_dataConf(NWK_DataReq_t *req) {
  dataReqBusy = false;
  (void)req;
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

#if DESIGNATION == 1  // client
  APP_dataSend(DISCOVER, 00);
#endif
}
