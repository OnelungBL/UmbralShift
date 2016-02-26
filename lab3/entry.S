//***********************************
//NAME: Darren Takemoto
//Phase 0, Lab 3 - Timer Lab
//entry.S
//***********************************
#include <spede/machine/asmacros.h>
#include "entry.h"

ENTRY(TimerEntry)		// push eflag, cs, eip (by circuit)
	pusha			// push all: e-acdb-x, e-sb-p, e-sd-i
	cld			// clear direction flag
	call CNAME(TimerISR)	// call C function TimerISR()
	popa			// pop all: e-ds-i, e-bs-p, e-bdca-x
	iret			// pop eip, cs, eflags
