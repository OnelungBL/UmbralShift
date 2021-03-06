// entry.S, 159
// entry points of interrupts

#include <spede/machine/asmacros.h>
#include "entry.h"

// set DS and ES to Kernels data segment
#define SET_KERNEL_SEGS movw $(KERNEL_DATA), %ax; mov %ax, %ds; mov %ax, %es

.comm kernel_stack, KERNEL_STACK_SIZE, 1 // define kernel stack space
.text

// Loader loads trapframe of a process to CPU registers, to run it
//(TF size becomes 68)
ENTRY(LoadRun)
   movl 4(%esp), %eax   // load stack pointer from eax
   movl %eax, %esp
   popl %gs
   popl %fs
   popl %es
   popl %ds
   popa                 // restore general registers
   add $8, %esp         // skip 4 bytes that stored intr_id, skip 4 bytes that stored err_no
   iret

// push intr type number then jump to common handler
ENTRY(TimerEntry)
   pushl $0
   pushl $TIMER_INTR
   jmp CompleteTFenterKernel

ENTRY(IRQ3Entry)
   pushl $0
   pushl $IRQ3_INTR
   jmp CompleteTFenterKernel

ENTRY(IRQ7Entry)
   pushl $0
   pushl $IRQ7_INTR
   jmp CompleteTFenterKernel

ENTRY(GetPidEntry)
   pushl $0
   pushl $GETPID_INTR
   jmp CompleteTFenterKernel

ENTRY(SleepEntry)
   pushl $0
   pushl $SLEEP_INTR
   jmp CompleteTFenterKernel

ENTRY(StartProcEntry)
   pushl $0
   pushl $STARTPROC_INTR
   jmp CompleteTFenterKernel

ENTRY(SemGetEntry)
   pushl $0
   pushl $SEMGET_INTR
   jmp CompleteTFenterKernel
  
ENTRY(SemWaitEntry)
   pushl $0
   pushl $SEMWAIT_INTR
   jmp CompleteTFenterKernel
  
ENTRY(SemPostEntry)
   pushl $0
   pushl $SEMPOST_INTR
   jmp CompleteTFenterKernel
  
ENTRY(MsgSndEntry)
   pushl $0
   pushl $MSGSND_INTR
   jmp CompleteTFenterKernel

ENTRY(MsgRcvEntry)
   pushl $0
   pushl $MSGRCV_INTR
   jmp CompleteTFenterKernel

ENTRY(ForkEntry)
   pushl $0
   pushl $FORK_INTR
   jmp CompleteTFenterKernel 

ENTRY(WaitEntry)
   pushl $0
   pushl $WAIT_INTR
   jmp CompleteTFenterKernel

ENTRY(ExitEntry)
   pushl $0
   pushl $EXIT_INTR
   jmp CompleteTFenterKernel

ENTRY(PFEntry)
  pushl $PF_INTR
  jmp CompleteTFenterKernel

//PF throws error number onto stack
CompleteTFenterKernel:
   pusha
   pushl %ds
   pushl %es
   pushl %fs
   pushl %gs
   movl %esp, %edx
   cld
   SET_KERNEL_SEGS      // set kernel data segments
   leal kernel_stack + KERNEL_STACK_SIZE, %esp
   pushl %edx
   call CNAME(KernelMain)
