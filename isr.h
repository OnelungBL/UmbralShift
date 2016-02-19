// isr.h, 159

#ifndef _ISR_H_
#define _ISR_H_

#include "typedef.h" // q_t needs be defined in code below

void StartProcISR(int new_pid, q_t *ready_q);
void EndProcISR(int new_pid, q_t *free_q);
void TimerISR(int new_pid, q_t *ready_q);

#endif
