#define BAUD 38400

#include "makra.h"
#include "uart/uart.h"
#include "uart/timer.h"
#include<stdbool.h>

uint8_t duty = 10;

int main(void) {
	bool timer0 = false;
	bool timer1 = false;
	bool timer2 = false;
	DDRB |= (1 << DDB4) | (1 << DDB5) | (1 << DDB6);
	DDRE |= (1 << DDE3);
	UART_Init(BAUD);
	
	UART_SendString("MENU:\r\n");
	UART_SendString("1: abeceda\r\n");
	UART_SendString("2: ABECEDA\r\n");
	UART_SendString("4: Timer0 ON\r\n");
	UART_SendString("5: Timer1 ON\r\n");
	UART_SendString("+: LED3 brightness up\r\n");
	UART_SendString("-: LED3 brightness down\r\n");

	
	
	while (1) {	
		char recv = UART_GetChar();
		switch (recv)
		{
		case '1':
			UART_SendString("abcdefghijklmnopqrstuvwxyz\r\n");
			break;
		case '2':
			UART_SendString("ABCDEFGHIJKLMNOPQRSTUVWXYZ\r\n");
			break;
		case '5':
			if (timer1 == false) {
				timer1 = true;
				Timer1_cmp_start();
			} else {
				timer1 = false;
				Timer1_end();
			}
			break;
		case '+':
		if (duty > 10) {
				duty -= 10; 
			}
			Timer2_fastpwm_start(duty);
			break;
		case '-':
			if (duty < 100) {
				 duty += 10; 
			}
			Timer2_fastpwm_start(duty);
			break;
		default:
			UART_SendString(strcat(recv, " unknown sequence\r\n"));
			break;
		}
	}
}
