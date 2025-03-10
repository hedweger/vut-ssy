#define BAUD 38400

#include "makra.h"
#include "libs/uart.h"
#include "libs/timer.h"
#include "libs/at30.h"
#include <stdbool.h>
#include <stdio.h>

uint8_t duty = 10;

int main(void) {
	bool timer0 = false;
	bool timer1 = false;
	bool timer2 = false;
	uint8_t res;
	uint16_t light; 
	float temp;
	char out_str[30];
	UART_Init(BAUD);
	cbi(DDRE, PORTE5); 
	sbi(EICRB, ISC51);
	cbi(EICRB, ISC50);
	sbi(EIMSK, INT5);
	sei(); 
	DDRB |= (1 << DDB4) | (1 << DDB5) | (1 << DDB6);
	DDRE |= (1 << DDE3);
	
	UART_SendString("MENU:\r\n");
	UART_SendString("1: abeceda\r\n");
	UART_SendString("2: ABECEDA\r\n");
	UART_SendString("4: Timer0 ON\r\n");
	UART_SendString("5: Timer1 ON\r\n");
	UART_SendString("+: LED3 brightness up\r\n");
	UART_SendString("-: LED3 brightness down\r\n");

	
	
	while (1) {	
		char recv = UART_GetChar();
		UART_SendString("Your input is: ");
		UART_SendChar(recv);
		UART_SendChar('\r');
		UART_SendChar('\n');
		switch (recv)
		{
		case '1':
			UART_SendString("abcdefghijklmnopqrstuvwxyz\r\n");
			break;
		case '2':
			UART_SendString("ABCDEFGHIJKLMNOPQRSTUVWXYZ\r\n");
			break;
		case '4':
			ADC_init(0x04,0x02);
			light = ADC_get(3);
			sprintf(out_str, "light intensity is: %d\r\n", light);
			UART_SendString(out_str);
			break;
		case 't':
			i2c_init();
			//res = at30_set_precision();
			temp = at30_read_temp();
			sprintf(out_str, "%f\r\n", temp);
			UART_SendString("Temperature is: ");
			UART_SendString(out_str);
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
			UART_SendString("unknown sequence\r\n");
			break;
		}
	}
}
