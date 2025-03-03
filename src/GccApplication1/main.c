#include "makra.h"
#include "uart/uart.h"

#define BAUD 38400

int main(void) {
  DDRB |= (1 << DDB5) | (1 << DDB6); // Set PORTB pins 5 and 6 as output
  DDRE |= (1 << DDE3);               // Set PORTE pin 3 as output
  LED1OFF;
  LED2OFF;
  LED3OFF;
  UART_Init(BAUD);
  UART_SendString("MENU:");
  UART_SendString("1: abeceda");
  UART_SendString("2: ABECEDA");
  UART_SendString("3: LED2 blik");
  while (1) {
    uint8_t recv = UART_GetChar();
    if (recv == 49) {
      UART_SendString("abcdefghijklmnopqrstuvwxyz\r\n");
    } else if (recv == 50) {
      UART_SendString("ABCDEFGHIJKLMNOPQRSTUVWXYZ\r\n");
    } else if (recv == 51) {
      LED2CHANGE;
      LED2CHANGE;
      LED2CHANGE;
    } else if (recv != 0) {
      UART_SendString("unknown sequence\r\n");
    }
  }
}
