//***********************************
//NAME: Darren Takemoto
//Phase 0, Lab 3 - Timer Lab
//entry.h of entry.S
//***********************************
#ifndef _ENTRY_H_

#define _ENTRY_H_
#define TIMER_INTR 32

#ifndef ASSEMBLER // skip if ASSEMBLER define (in assembly code)
void TimerEntry();  // defined in entry.S, assembly won't take this
#endif

#endif
