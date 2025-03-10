#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
//FILE uart_str = FDEV_SETUP_STREAM(printCHAR, NULL, _FDEV_SETUP_RW);

void UART_Init(uint16_t baudrate) {
  //stdout = &uart_str;
  int ubbr = (F_CPU / 16 / baudrate) - 1;
  UBRR1H = (uint8_t)(ubbr >> 8);
  UBRR1L = (uint8_t)ubbr;
  UCSR1B = (1 << RXEN1) | (1 << TXEN1);
}

void UART_SendChar(uint8_t data) {
  while (!(UCSR1A & (1 << UDRE1)));
  UDR1 = data;
}

void UART_SendString(char *text) {
    while (*text != 0x00) { 
	    UART_SendChar(*text);
	    text++;
    }
}

uint8_t UART_GetChar(void) {
  while (!(UCSR1A & (1 << RXC1)));
  return UDR1;
}
