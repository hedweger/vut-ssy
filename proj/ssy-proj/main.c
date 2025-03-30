/*
 * LWM_MSSY_deRF.c
 *
 * Created: 14.4.2017 12:56:52
 * Author : Ondra
 */ 

#include <avr/io.h>
/*- Includes ---------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "config.h"
#include "hal.h"
#include "phy.h"
#include "sys.h"
#include "nwk.h"
#include "sysTimer.h"
#include "halBoard.h"
#include "halUart.h"
#include "util/delay.h"
#include "ADC_lib.h"
#include "main.h"
#include "derf/sensors_interface.h"
#include "commands.h"
#include "halUart.h"
#include "halSleep.h"
#include "halBoard.h"
#include "halLed.h"
#include "derf/usb.h"

/*- Definitions ------------------------------------------------------------*/
#if defined(APP_COORDINATOR)
  #define APP_NODE_TYPE     0
#elif defined(APP_ROUTER)
  #define APP_NODE_TYPE     1
#else
  #define APP_NODE_TYPE     2
#endif

#define APP_CAPTION_SIZE    (sizeof(APP_CAPTION) - 1)
#define APP_COMMAND_PENDING 0x01

#define APP_ENDPOINT        1

#define APP_LED_NETWORK     0
#define APP_LED_DATA        1



#define ADDR_REQUEST_MSG 0x10
#define ADDR_RESPONSE_MSG 0x20
#define ADDR_CONFIRM_MSG 0x30

/*- Types ------------------------------------------------------------------*/
typedef struct PACK
{
  uint8_t      commandId;
  uint8_t      nodeType;
  uint64_t     extAddr;
  uint16_t     shortAddr;
  uint32_t     softVersion;
  uint32_t     channelMask;
  uint8_t      workingChannel;
  uint16_t      panId;
  uint16_t    parentShortAddr;
  uint8_t      lqi;
  int8_t       rssi;

  struct PACK
  {
    uint8_t    type;
    uint8_t    size;
    int32_t    battery;
    int32_t    temperature;
    int32_t    light;
  } sensors;

  struct PACK
  {
    uint8_t    type;
    uint8_t    size;
    char       text[APP_CAPTION_SIZE];
  } caption;
} AppMessage_t; 


typedef struct PACK
{
uint8_t msg_ID;
uint16_t node_address;
uint16_t node_ID;
}AppAddress_t;

typedef enum AppState_t
{
  APP_STATE_INITIAL,
  APP_STATE_ADDR_REQUEST,
  APP_STATE_ADDR_WAIT,
  APP_STATE_SEND,
  APP_STATE_WAIT_CONF,
  APP_STATE_SENDING_DONE,
  APP_STATE_WAIT_SEND_TIMER,
  APP_STATE_WAIT_COMMAND_TIMER,
  APP_STATE_PREPARE_TO_SLEEP,
  APP_STATE_SLEEP,
  APP_STATE_WAKEUP,
} AppState_t;

int printCHAR(char character, FILE *stream);
static void appAddrRestponse(uint16_t src_addr,uint16_t node_id);
static void appAddrRequest(uint16_t node_id);
static void appAddrConf(uint16_t my_addr,uint16_t node_id);
static void appADDR_REQ_Conf(NWK_DataReq_t *req);
/*- Variables --------------------------------------------------------------*/
static AppState_t appState = APP_STATE_INITIAL;
//FILE uart_str = FDEV_SETUP_STREAM(printCHAR, NULL, _FDEV_SETUP_RW);//soubor pro stdout

FILE usb_stream = FDEV_SETUP_STREAM(usb_putc_std, usb_getc_std,_FDEV_SETUP_RW);


//#if defined(APP_ROUTER) || defined(APP_ENDDEVICE)
static NWK_DataReq_t appNwkDataReq;
static SYS_Timer_t appNetworkStatusTimer;
static SYS_Timer_t appCommandWaitTimer;
static bool appNetworkStatus;
//#endif

static AppMessage_t appMsg;
static SYS_Timer_t appDataSendingTimer;

uint16_t myNodeID = 0;
uint32_t messno=0;

/*- Implementations --------------------------------------------------------*/

/*************************************************************************/
/*
int printCHAR(char character, FILE *stream)
{
	HAL_UartWriteByte(character);

	return 0;
}
*/
/**
*****************************************************************************/
void HAL_UartBytesReceived(uint16_t bytes)
{
  for (uint16_t i = 0; i < bytes; i++)
  {
    uint8_t byte = HAL_UartReadByte();
    APP_CommandsByteReceived(byte);
  }
}

/*************************************************************************//**
*****************************************************************************/
static void appUartSendMessage(uint8_t *data, uint8_t size)
{
  
  uint8_t cmd_buff[127];
  uint8_t cmd_buff_pos=0;
  uint8_t cs = 0;
  
/*
  HAL_UartWriteByte(0x10);
  HAL_UartWriteByte(0x02);

  for (uint8_t i = 0; i < size; i++)
  {
    if (data[i] == 0x10)
    {
      HAL_UartWriteByte(0x10);
      cs += 0x10;
    }
    HAL_UartWriteByte(data[i]);
    cs += data[i];
  }

  HAL_UartWriteByte(0x10);
  HAL_UartWriteByte(0x03);
  cs += 0x10 + 0x02 + 0x10 + 0x03;

  HAL_UartWriteByte(cs);
  */
 cmd_buff[cmd_buff_pos++]=0x10;
 cmd_buff[cmd_buff_pos++]=0x02;

  for (uint8_t i = 0; i < size; i++)
  {
	  if (data[i] == 0x10)
	  {
		  cmd_buff[cmd_buff_pos++]=0x10;
		  cs += 0x10;
	  }
	  cmd_buff[cmd_buff_pos++]=data[i];
	  cs += data[i];
  }

  cmd_buff[cmd_buff_pos++]=0x10;
  cmd_buff[cmd_buff_pos++]=0x03;
  cs += 0x10 + 0x02 + 0x10 + 0x03;
  cmd_buff[cmd_buff_pos++]=cs;
 
printf("%.*s", cmd_buff_pos, cmd_buff);
}
static void appUartSendMessageHR(uint8_t *data, uint8_t size)
{
	AppMessage_t *BufferHR = (AppMessage_t *)data;
	//static uint8_t BufferHR_size = size;
	
	
	if (BufferHR->shortAddr!=0)
	{
	printf("----------------------\n\r");
	//printf("Command ID: %d \n\r", BufferHR->commandId);
	printf("Node Type: %d \n\r", BufferHR->nodeType);
	//printf("Ext addr: %llu \n\r", BufferHR->extAddr);
	//printf("Short addr: %hu \n\r", BufferHR->shortAddr);
	printf("Short addr: 0x%X \n\r", BufferHR->shortAddr);
	//printf("Soft ver.: %lu \n\r", BufferHR->softVersion);
	//printf("Channel mask: %lu \n\r", BufferHR->channelMask);
	//printf("Working channel: %d \n\r", BufferHR->workingChannel);
	//printf("Pan ID: %hu \n\r", BufferHR->panId);
	printf("Parent addr: 0x%X \n\r", BufferHR->parentShortAddr);
	//printf("Parent addr: %hu \n\r", BufferHR->parentShortAddr);
	printf("LQI: %d \n\r", BufferHR->lqi);
	printf("RSSI: %d \n\r", BufferHR->rssi);
	printf("Node Type: %d \n\r", BufferHR->nodeType);
	printf("Sensors Type: %d \n\r", BufferHR->sensors.type);
	printf("Sensors Size: %d \n\r", BufferHR->sensors.size);
	printf("Sensors Battery: %d \n\r", BufferHR->sensors.battery);
	printf("Sensors Temp: %d \n\r", BufferHR->sensors.temperature);
	printf("Sensors Light: %d \n\r", BufferHR->sensors.light);
	printf("Caption Type: %d \n\r", BufferHR->caption.type);
	printf("Caption Size: %d \n\r", BufferHR->caption.size);
	printf("Caption Text:");
	int i;
	for (i=0;i<BufferHR->caption.size;i++)
	{
		printf("%c",(char)BufferHR->caption.text[i]);
		//HAL_UartWriteByte(BufferHR->caption.text[i]);
	}
	//printf("%.*s", BufferHR->caption.size, BufferHR->caption.text);
	printf("\n\r");
	}
	else{
		
	}

}

/*************************************************************************//**
*****************************************************************************/
static bool appDataInd(NWK_DataInd_t *ind)
{
  AppMessage_t *msg = (AppMessage_t *)ind->data; 
  //not whole message, but payload only

  HAL_LedToggle(APP_LED_DATA);

  msg->lqi = ind->lqi;
  msg->rssi = ind->rssi;
  
  //appUartSendMessage(ind->data, ind->size);
  appUartSendMessageHR(ind->data, ind->size);

  if (APP_CommandsPending(ind->srcAddr))
    NWK_SetAckControl(APP_COMMAND_PENDING);

  return true;
}
static bool appAddrInd(NWK_DataInd_t *ind)
{
	printf("Address message \n\r");
	AppAddress_t *addr_msg = (AppAddress_t *)ind->data;
	
	switch (addr_msg->msg_ID)
	{
	#if defined(APP_COORDINATOR)
	case ADDR_REQUEST_MSG:
	
	appAddrRestponse(ind->srcAddr,addr_msg->node_ID);
	printf("Address request received from node ID %x \n\r",addr_msg->node_ID);
	
	break;
		
	case ADDR_CONFIRM_MSG:
	printf("Address %x accepted by node ID %x \n\r",addr_msg->node_address,addr_msg->node_ID);
	break;
	
	#endif
	
	#if defined(APP_ROUTER) || defined(APP_ENDDEVICE)
	
	case ADDR_RESPONSE_MSG:
	printf("Address response rcvd, %x \r",addr_msg->node_address);
	appMsg.extAddr              = addr_msg->node_address;
	appMsg.shortAddr            = addr_msg->node_address;
	NWK_SetAddr(addr_msg->node_address);
	
	appAddrConf(addr_msg->node_address,myNodeID);
	
	//appState = APP_STATE_ADDR_WAIT;
	
	break;
		
	#endif
	
	default :
	printf("Unknown Address Message:  %x /n/r",addr_msg->msg_ID);		
	}
	
	return true;
}

/*************************************************************************//**
*****************************************************************************/
static void appDataSendingTimerHandler(SYS_Timer_t *timer)
{
  if (APP_STATE_WAIT_SEND_TIMER == appState)
    appState = APP_STATE_SEND;
  else
    SYS_TimerStart(&appDataSendingTimer);

  (void)timer;
}

#if defined(APP_ROUTER) || defined(APP_ENDDEVICE)
/*************************************************************************//**
*****************************************************************************/
static void appNetworkStatusTimerHandler(SYS_Timer_t *timer)
{
  HAL_LedToggle(APP_LED_NETWORK);
  (void)timer;
}

/*************************************************************************//**
*****************************************************************************/
static void appCommandWaitTimerHandler(SYS_Timer_t *timer)
{
  appState = APP_STATE_SENDING_DONE;
  (void)timer;
}
#endif

/*************************************************************************//**
*****************************************************************************/
//#if defined(APP_ROUTER) || defined(APP_ENDDEVICE)
static void appDataConf(NWK_DataReq_t *req)
{
  HAL_LedOff(APP_LED_DATA);

  if (NWK_SUCCESS_STATUS == req->status)
  {
    if (!appNetworkStatus)
    {
      HAL_LedOn(APP_LED_NETWORK);
      SYS_TimerStop(&appNetworkStatusTimer);
      appNetworkStatus = true;
    }
  }
  else
  {
    if (appNetworkStatus)
    {
      HAL_LedOff(APP_LED_NETWORK);
      SYS_TimerStart(&appNetworkStatusTimer);
      appNetworkStatus = false;
    }
  }

  if (APP_COMMAND_PENDING == req->control)
  {
    SYS_TimerStart(&appCommandWaitTimer);
    appState = APP_STATE_WAIT_COMMAND_TIMER;
  }
  else
  {
	  if (appState != APP_STATE_ADDR_WAIT)
	  {
		  appState = APP_STATE_SENDING_DONE;
	  }
    
  }
}
//#endif
static void appADDR_REQ_Conf(NWK_DataReq_t *req)
{
  appState = APP_STATE_ADDR_WAIT;
}
static void appADDR_CONF_Conf(NWK_DataReq_t *req)
{
  appState = APP_STATE_SEND;
}
//#endif

/*************************************************************************//**
*****************************************************************************/
static void appSendData(void)
{
#ifdef NWK_ENABLE_ROUTING
  appMsg.parentShortAddr = NWK_RouteNextHop(0, 0);
#else
  appMsg.parentShortAddr = 0;
#endif

  appMsg.sensors.battery     = rand() & 0xffff;
  ADC_Init(4,2);
  appMsg.sensors.temperature = ADC_readTemp();
  appMsg.sensors.light       = messno++;//ADC_get(3);

#if defined(APP_COORDINATOR)
  appUartSendMessageHR((uint8_t *)&appMsg, sizeof(appMsg));
  SYS_TimerStart(&appDataSendingTimer);
  appState = APP_STATE_WAIT_SEND_TIMER;
#else
  appNwkDataReq.dstAddr = 0;
  appNwkDataReq.dstEndpoint = APP_ENDPOINT;
  appNwkDataReq.srcEndpoint = APP_ENDPOINT;
  appNwkDataReq.options = NWK_OPT_ACK_REQUEST | NWK_OPT_ENABLE_SECURITY;
  appNwkDataReq.data = (uint8_t *)&appMsg;
  appNwkDataReq.size = sizeof(appMsg);
  appNwkDataReq.confirm = appDataConf;

  HAL_LedOn(APP_LED_DATA);
  NWK_DataReq(&appNwkDataReq);

  appState = APP_STATE_WAIT_CONF;
#endif
}

static void appAddrRestponse(uint16_t src_addr,uint16_t node_id)
{
	static AppAddress_t msg;
	msg.node_ID=node_id;
	msg.msg_ID=ADDR_RESPONSE_MSG;
	msg.node_address=0x6000;
	
	
	appNwkDataReq.dstAddr = src_addr;
	appNwkDataReq.dstEndpoint = 2;
	appNwkDataReq.srcEndpoint = 2;
	appNwkDataReq.options = NWK_OPT_ACK_REQUEST | NWK_OPT_ENABLE_SECURITY;
	appNwkDataReq.data = (uint8_t *)&msg;
	appNwkDataReq.size = sizeof(msg);
	appNwkDataReq.confirm = appDataConf;

	HAL_LedOn(APP_LED_DATA);
	NWK_DataReq(&appNwkDataReq);

	appState = APP_STATE_WAIT_CONF;
	
}
static void appAddrRequest(uint16_t node_id)
{
	static AppAddress_t msg;
	msg.node_ID=node_id;
	msg.msg_ID=ADDR_REQUEST_MSG;
	msg.node_address=APP_ADDR;
	
	
	appNwkDataReq.dstAddr = 0x0000;
	appNwkDataReq.dstEndpoint = 2;
	appNwkDataReq.srcEndpoint = 2;
	appNwkDataReq.options = NWK_OPT_ACK_REQUEST | NWK_OPT_ENABLE_SECURITY;
	appNwkDataReq.data = (uint8_t *)&msg;
	appNwkDataReq.size = sizeof(msg);
	appNwkDataReq.confirm = appADDR_REQ_Conf;

	HAL_LedOn(APP_LED_DATA);
	NWK_DataReq(&appNwkDataReq);

	appState = APP_STATE_ADDR_WAIT;
	
}
static void appAddrConf(uint16_t my_addr,uint16_t node_id)
{
	static AppAddress_t msg;
	msg.node_ID=node_id;
	msg.msg_ID=ADDR_CONFIRM_MSG;
	msg.node_address=my_addr;
	
	
	appNwkDataReq.dstAddr = 0x0000;
	appNwkDataReq.dstEndpoint = 2;
	appNwkDataReq.srcEndpoint = 2;
	appNwkDataReq.options = NWK_OPT_ACK_REQUEST | NWK_OPT_ENABLE_SECURITY;
	appNwkDataReq.data = (uint8_t *)&msg;
	appNwkDataReq.size = sizeof(msg);
	appNwkDataReq.confirm = appADDR_CONF_Conf;

	HAL_LedOn(APP_LED_DATA);
	NWK_DataReq(&appNwkDataReq);

	appState = APP_STATE_WAIT_CONF;
	
}

/*************************************************************************//**
*****************************************************************************/
static void appInit(void)
{
  appMsg.commandId            = APP_COMMAND_ID_NETWORK_INFO;
  appMsg.nodeType             = APP_NODE_TYPE;
  appMsg.extAddr              = APP_ADDR;
  appMsg.shortAddr            = APP_ADDR;
  appMsg.softVersion          = 0x01010100;
  appMsg.channelMask          = (1L << APP_CHANNEL);
  appMsg.panId                = APP_PANID;
  appMsg.workingChannel       = APP_CHANNEL;
  appMsg.parentShortAddr      = 0;
  appMsg.lqi                  = 0;
  appMsg.rssi                 = 0;

  appMsg.sensors.type        = 1;
  appMsg.sensors.size        = sizeof(int32_t) * 3;
  appMsg.sensors.battery     = 0;
  appMsg.sensors.temperature = 0;
  appMsg.sensors.light       = 0;

  appMsg.caption.type         = 32;
  appMsg.caption.size         = APP_CAPTION_SIZE;
  memcpy(appMsg.caption.text, APP_CAPTION, APP_CAPTION_SIZE);

  HAL_BoardInit();
  HAL_LedInit();

  NWK_SetAddr(APP_ADDR);
  NWK_SetPanId(APP_PANID);
  PHY_SetChannel(APP_CHANNEL);
#ifdef PHY_AT86RF212
  PHY_SetBand(APP_BAND);
  PHY_SetModulation(APP_MODULATION);
#endif
  PHY_SetRxState(true);

#ifdef NWK_ENABLE_SECURITY
  NWK_SetSecurityKey((uint8_t *)APP_SECURITY_KEY);
#endif

  NWK_OpenEndpoint(APP_ENDPOINT, appDataInd);
  NWK_OpenEndpoint(2, appAddrInd);

  appDataSendingTimer.interval = APP_SENDING_INTERVAL;
  appDataSendingTimer.mode = SYS_TIMER_INTERVAL_MODE;
  appDataSendingTimer.handler = appDataSendingTimerHandler;

#if defined(APP_ROUTER) || defined(APP_ENDDEVICE)
  appNetworkStatus = false;
  appNetworkStatusTimer.interval = 500;
  appNetworkStatusTimer.mode = SYS_TIMER_PERIODIC_MODE;
  appNetworkStatusTimer.handler = appNetworkStatusTimerHandler;
  SYS_TimerStart(&appNetworkStatusTimer);

  appCommandWaitTimer.interval = NWK_ACK_WAIT_TIME;
  appCommandWaitTimer.mode = SYS_TIMER_INTERVAL_MODE;
  appCommandWaitTimer.handler = appCommandWaitTimerHandler;
#else
  HAL_LedOn(APP_LED_NETWORK);
#endif

#ifdef PHY_ENABLE_RANDOM_NUMBER_GENERATOR
  srand(PHY_RandomReq());
#endif

  APP_CommandsInit();

#if defined(APP_ROUTER) || defined(APP_ENDDEVICE)
	appState = APP_STATE_ADDR_REQUEST;
#else
	appState = APP_STATE_SEND;
#endif
  

  ADC_Init(4,2);
}

/*************************************************************************//**
*****************************************************************************/
static void APP_TaskHandler(void)
{
  switch (appState)
  {
    case APP_STATE_INITIAL:
    {
      appInit();
    } break;

    case APP_STATE_SEND:
    {
      appSendData();
    } break;
	
	case APP_STATE_ADDR_REQUEST:
	{
	   myNodeID = rand() & 0xffff;
	   appAddrRequest(myNodeID);
	   printf("ADDR_req, nodeID=%x \n\r",myNodeID);
	   
	}break;
	case APP_STATE_ADDR_WAIT:
	{
		
	}break;
	case APP_STATE_WAIT_CONF:
	{
		
	}break;

    case APP_STATE_SENDING_DONE:
    {
#if defined(APP_ENDDEVICE)
      appState = APP_STATE_PREPARE_TO_SLEEP;
#else
      SYS_TimerStart(&appDataSendingTimer);
      appState = APP_STATE_WAIT_SEND_TIMER;
#endif
    } break;

    case APP_STATE_PREPARE_TO_SLEEP:
    {
      if (!NWK_Busy())
      {
        NWK_SleepReq();
        appState = APP_STATE_SLEEP;
      }
    } break;

    case APP_STATE_SLEEP:
    {
      HAL_LedClose();
      HAL_Sleep(APP_SENDING_INTERVAL);
      appState = APP_STATE_WAKEUP;
    } break;

    case APP_STATE_WAKEUP:
    {
      NWK_WakeupReq();

      HAL_LedInit();
      HAL_LedOn(APP_LED_NETWORK);

      appState = APP_STATE_SEND;
    } break;

    default:
      break;
  }
}

/*************************************************************************//**
*****************************************************************************/
int main(void)
{
  SYS_Init();
  //HAL_UartInit(38400);
  //stdout = &uart_str;
  #if defined(APP_COORDINATOR)
  usb_init();
  _delay_ms(500);
  stdout = &usb_stream;
  stdin  = &usb_stream;
  _delay_ms(500);
  #endif
  printf("Start... \n\r");

  while (1)
  {
    SYS_TaskHandler();
    HAL_UartTaskHandler();
    APP_TaskHandler();
  }
}
