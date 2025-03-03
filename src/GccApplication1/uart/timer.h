#ifndef TIMER_H_
#define TIMER_H_



void Timer1_cmp_start();
void Timer2_fastpwm_start(uint8_t duty);
void Timer0_ovf_start();
void Timer0_end();
void Timer1_end();
void Timer2_end();

#endif /* TIMER_H_ */
