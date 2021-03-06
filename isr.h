// isr.h, 159

#ifndef _ISR_H_
#define _ISR_H_

#include "typedef.h"
#include "FileService.h"

void StartProcISR(int new_pid, int func_addr);
void TimerISR();
void GetPidISR();
void SleepISR();

void SemGetISR(int);
void SemWaitISR(int);
void SemPostISR(int);

void MsgSndISR(int);
void MsgRcvISR(int);

void IRQ3ISR();
void TX();
void RX();

void ForkISR(int, int);
void WaitISR();
void ExitISR();
void PF_ISR();
#endif
