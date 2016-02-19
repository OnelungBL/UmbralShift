// isr.h, 159

#ifndef _ISR_H_
#define _ISR_H_

#include "typedef.h" // q_t needs be defined in code below

void StartProcISR(int, q_t *);
void EndProcISR(int, q_t *);
void TimerISR(int, q_t *);

#endif
