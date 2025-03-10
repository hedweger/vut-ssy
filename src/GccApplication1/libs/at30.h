/*
 * at30.h
 *
 * Created: 3/10/2025 11:46:02
 *  Author: Student
 */ 


#ifndef AT30_H_
#define AT30_H_
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#include "../makra.h"
#include "i2c.h"
#define TempSensorAddrR 0b10010111
#define SerialEEPROMAddrR 0b10100111
#define TempSensorAddrW 0b10010110
#define SerialEEPROMAddrW 0b10100110
#define OS 15
#define R1 14
#define R0 13
#define FT1 12
#define FT0 11
#define POL 10
#define CMPINT 9
#define SD 9
#define NVRBSY 0
#define Temp_tempRegister 0x00
#define Temp_configRegister 0x01
#define Temp_lowLimRegister 0x02
#define Temp_highLimRegister 0x03

uint8_t at30_set_precision(uint8_t precision);
float at30_read_temp(void);



#endif /* AT30_H_ */