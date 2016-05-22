// entry.h, 159

#ifndef _ENTRY_H_
#define _ENTRY_H_

#include <spede/machine/pic.h>

#define PF_INTR 14
#define TIMER_INTR 32
#define IRQ3_INTR 35
#define IRQ7_INTR 39
#define GETPID_INTR 48
#define SLEEP_INTR 49
#define STARTPROC_INTR 50
#define SEMGET_INTR 51
#define SEMWAIT_INTR 52
#define SEMPOST_INTR 53
#define MSGSND_INTR 54
#define MSGRCV_INTR 55
#define FORK_INTR 56
#define WAIT_INTR 57
#define EXIT_INTR 58

#define KERNEL_CODE 0x08         // kernel's code segment
#define KERNEL_DATA 0x10         // kernel's data segment
#define KERNEL_STACK_SIZE 32768  // kernel's stack byte size

#ifndef ASSEMBLER

__BEGIN_DECLS

#include "TF.h"

extern void LoadRun(TF_t *);     // code defined in entry.S
extern void TimerEntry();        // code defined in entry.S

extern void IRQ3Entry();
extern void IRQ7Entry();

extern void GetPidEntry();      // code defined in entry.S
extern void SleepEntry();       // code defined in entry.S
extern void StartProcEntry();

extern void SemGetEntry();
extern void SemWaitEntry();
extern void SemPostEntry();

extern void MsgSndEntry();
extern void MsgRcvEntry();

extern void ForkEntry();
extern void WaitEntry();
extern void ExitEntry();

extern void PFEntry();
__END_DECLS

#endif

#endif

