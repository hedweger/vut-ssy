/*
 * uart.h
 *
 * Created: 2/24/2025 12:14:45
 *  Author: Student
 */ 


#ifndef UART_H_
#define UART_H_


void UART_Init(uint16_t baudrate);
void UART_SendChar(uint8_t data);
void UART_SendString(char* text);
uint8_t UART_GetChar( void );



#endif /* UART_H_ */