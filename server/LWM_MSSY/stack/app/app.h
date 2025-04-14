#ifndef _APP_H_
#define _APP_H_

#include "phy.h"
#include <stdbool.h>
#include <stdint.h>

enum APP_MsgType_t {
  APP_ACK,
  APP_NACK,
  APP_DISCOVER,
  APP_OFFER,
  APP_REQUEST,
  APP_RELEASE,
};

enum APP_deviceType_t {
	LOW,
	HIGH,
};

typedef struct APP_AddrTrans_t {
  uint16_t PHY_addr;
  uint16_t APP_addr;
  bool ack;
} APP_AddrTrans_t;

APP_AddrTrans_t APP_addrTrans[999];

typedef struct APP_DataMsg_t {
  PHY_DataInd_t *PHY_data;
  enum APP_MsgType_t type;
} APP_DataMsg_t;

void APP_init(void);
void APP_addrAdd(uint16_t addr);
void APP_addrReq(void);


#endif
