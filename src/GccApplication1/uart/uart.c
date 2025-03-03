#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#define F_CPU 8000000 // 8 MHz internal clock

void UART_Init(uint16_t baudrate) {
  int ubbr = ((F_CPU / 16 / baudrate) - 1);
  UBRR1H = (uint8_t)(ubbr >> 8);
  UBRR1L = (uint8_t)ubbr;
  UCSR1B = (1 << RXCIE1) | (1 << TXCIE1) | (1 << RXEN1) | (1 << TXEN1);
}

void UART_SendChar(uint8_t data) {
  while (!(UCSR1A & (1 << UDRE1)))
    ;
  UDR1 = data;
}

void UART_SendString(char *text) {
  for (int i = 0; i < strlen(text); i++) {
    if (text[i] == 0x00) {
      return;
    }
    UART_SendChar(text[i]);
  }
}

uint8_t UART_GetChar(void) {
  while (!(UCSR1A & (1 << RXC1)))
    ;
  return UDR1;
}

ISR(USART1_RX_vect) {
  uint8_t recv;
  recv = UART_GetChar();
  if (recv == 1) {
    UART_SendChar('1');
  }
}
