#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "app.h"
#include "halUart.h"
#include "nwk.h"
#include "nwkDataReq.h"
#include "nwkRx.h"

static AppState_t appState;
static SYS_Timer_t timer;

static NWK_DataReq_t dataReq;
static bool dataReqBusy = false;
static uint8_t dataReqBuffer[NWK_MAX_PAYLOAD_SIZE];

static uint8_t uartBuffer[NWK_MAX_PAYLOAD_SIZE];
static uint8_t uartBufferPtr = 0;

static AppMsg_t appMsg;
static uint8_t *appMsgPointer = &appMsg;
static uint8_t appMsgBuffer[NWK_MAX_PAYLOAD_SIZE - sizeof(AppMsg_t)];

static RouteTable_t routeTable[ROUTE_TABLE_SIZE];
static uint8_t routeTablePtr = 0;

void HAL_UartBytesReceived(uint16_t bytes) { HAL_UartWriteString(bytes); }

void APP_dataSend(AppMsgType_t msgType, uint8_t addr) {
  RouteTable_t route = routeTable[addr];
  switch (msgType) {
  case OFFER:
    dataReq.data = &addr;
    dataReq.size = sizeof(uint8_t);
    break;
  case APPDATA:
    dataReq.data = &"data";
    dataReq.size = sizeof(dataReq.data);
    break;
  }

  dataReq.dstAddr = route.addr;
  dataReq.dstEndpoint = route.endpoint;
  dataReq.srcEndpoint = route.endpoint;
  dataReq.options = NWK_OPT_ENABLE_SECURITY;
  dataReq.confirm = APP_dataConf;
  NWK_DataReq(&dataReq);

  uartBufferPtr = 0;
  dataReqBusy = true;
}

/* Returns the index of the route table entry and pushes
 * the entry onto the table.
 * Should only be called when the server offers the config,
 * on accept from client, the in_use flag needs to be set,
 * otherwise the server will rewrite the entry on next pass.
 * The route table has 65535 entries
 */
uint16_t APP_pushAddr(uint8_t endpoint, uint8_t *data) {
  if (routeTable[routeTablePtr].in_use) {
    routeTablePtr++;
    return APP_pushAddr(endpoint, data);
  }

  routeTable[routeTablePtr].addr = routeTablePtr;
  routeTable[routeTablePtr].endpoint = endpoint;
  routeTable[routeTablePtr].sleeping = false;
  routeTable[routeTablePtr].type = (SensorType_t)(*data);
  // in_use only goes active on APP_ACK from client
  routeTable[routeTablePtr].in_use = false;
  return routeTablePtr;
}

bool APP_dataRecv(NWK_DataInd_t *ind) {
  AppMsg_t *recv = (AppMsg_t *)ind->data;
#if DESIGNATION == 1 // client
  switch (msgType) {
  case RELEASE:
    /*
     * Client handles the release of
     * its adress on its own.
     * The APP_ACK is no necessary, but
     * it seems like a good idea.
     */
    APP_dataSend(APP_ACK, ind->srcAddr);
    break;
  case OFFER:
    /*
     * Client handles the offer, then sends APP_ACK or
     * NAPP_ACK.
     * Ater NAPP_ACK, the server will send another
     * OFFER. The client shouldn't be evil and request
     * adresses unecessarily, even though nothing
     * bad could theoretically happen.
     */
    if (1 == 1) {
      APP_dataSend(APP_ACK, ind->srcAddr);
    }
    break;
  case REQUEST_DATA:
    /*
     * Client handles procuring data per its need
     */
    APP_dataSend(APPDATA, ind->srcAddr);
    break;
  }
#else
  switch (recv->msgType) {
  case APPDATA:
    routeTable[ind->srcAddr].sleeping = true;
    /*
     * Place to handle receiving data from clients.
     */
    for (int i = 0; i < recv->size; i++) {
      HAL_UartWriteByte(recv->data[i]);
    }
    break;
  case APP_SLEEP:
    routeTable[ind->srcAddr].sleeping = true;
    break;
  case RECONNECT:
    routeTable[ind->srcAddr].sleeping = false;
    break;
  case DISCOVER:
  case DECLINE:
    /*
     * Adresses are stored in routeTable,
     * on DISCOVER, the pointer is incremented
     * and stored in the table.
     * On DECLINE, the server does the same.
     * There is no limit for the amount of times
     * a client can refuse, I assume clients are
     * not evil.
     * Also, I presume that clients will not
     * request new adresses once they receive
     * one.
     */
    routeTablePtr++;
    int16_t addr = APP_pushAddr(ind->srcEndpoint, ind->data);
    APP_dataSend(OFFER, addr);
    break;
  case REQUEST:
    /*
     * REQUEST is a confirmation from the client,
     * that they accept the adress and will use it
     * in further communication.
     * Therefore, it is presumed the REQUEST message
     * will be sent with the server-assigned adress,
     * so the server can set their adress as in use
     * in the routing table.
     */
    routeTable[ind->srcAddr].in_use = true;
    HAL_UartWriteString("Accepted client on addr.: ");
    HAL_UartWriteString(ind->srcAddr);
    APP_dataSend(APP_ACK, ind->srcAddr);
    break;
  default:
    HAL_UartWriteString("Unknown msg type: ");
    HAL_UartWriteByte(recv->msgType);
    break;
  }
#endif

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
  dataReq.data = dataReqBuffer;

#if DESIGNATION == 1 // client
  APP_dataSend(DISCOVER, 00);
#endif
}
