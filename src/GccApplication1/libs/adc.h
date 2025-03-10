/*
 * adc.h
 *
 * Created: 3/10/2025 13:19:23
 *  Author: Student
 */ 


#ifndef ADC_H_
#define ADC_H_
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#include "../makra.h"
void ADC_init(uint8_t prescale, uint8_t uref);
uint16_t ADC_get(uint16_t chan);
void ADC_stop(void);




#endif /* ADC_H_ */