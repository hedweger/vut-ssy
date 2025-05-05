/*
 * appMsg.h
 *
 * Created: 5/5/2025 11:33:52
 *  Author: Student
 */ 


#ifndef APPMSG_H_
#define APPMSG_H_

typedef enum SensorType_t {
	WEIGHT_SENSOR,
	OTHER,
} SensorType_t;

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

typedef struct AppMsg_t {
	// header
	uint8_t size;
	AppMsgType_t msgType;
	uint8_t sleep;
	// data
	uint8_t *data;
} AppMsg_t;


#endif /* APPMSG_H_ */