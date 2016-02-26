// isr.h, 159

#ifndef _ISR_H_
#define _ISR_H_

#include "typedef.h" // q_t needs be defined in code below

void StartProcISR(int, q_t *, pcb_t *);
void EndProcISR(q_t *, pcb_t *);
void TimerISR(q_t *, pcb_t *);

#endif
