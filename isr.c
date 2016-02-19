// isr.c, 159

#include "spede.h"
#include "typedef.h"
#include "isr.h"
#include "toolfunc.h"
#include "extern.h"
#include "proc.h"
#include <spede/flames.h>
#include <spede/machine/io.h>
#include <spede/machine/pic.h>

void StartProcISR(int new_pid, q_t *ready_q, pcb_t *pcb) {
  //How to clear the PCB of a new pid?  -- clear the PCB of the new pid
  pcb[new_pid].runtime = 0;
  pcb[new_pid].total_runtime = 0;
  pcb[new_pid].state = READY; //set its state to READY
  
  if (new_pid != 0) {
  	EnQ(new_pid, ready_q);
  }
  printf("New proc! Pid: %d\n",new_pid);
}

void EndProcISR(int running_pid, q_t *free_q, pcb_t *pcb) {
   printf("In EndProcISR: Pid: %d\n",running_pid);
   if(running_pid == 0){ //if running PID is 0 (IdleProc should not let exit),
      return; //then, just return;
   } else {
   	pcb[running_pid].state=FREE;
   	EnQ(running_pid, free_q);
   	running_pid=-1;
   }
   //change state of running process to FREE
   //queue the running PID to free queue
   //set running PID to -1 (now none)
}        

void TimerISR(int *running_pid, q_t *ready_q, pcb_t *pcb) {
   if(*running_pid==-1){ //just return if running PID is -1 (not any valid PID)
        cons_printf("Uh oh!");  //There was a problem!
	return;
   }
   //(shouldn't happen, a Panic message can be considered)
   
   outportb(0x20, 0x60);
   //in PCB, upcount both runtime and total_runtime of running process
   pcb[*running_pid].runtime++;
   pcb[*running_pid].total_runtime++;
printf("runPid: %d, pcb runtime: %d, total runtime: %d\n", *running_pid, pcb[*running_pid].runtime, pcb[*running_pid].total_runtime);
   if (pcb[*running_pid].runtime >= TIME_LIMIT) {
   	pcb[*running_pid].runtime = 0;
   	pcb[*running_pid].state=READY;
   	EnQ(*running_pid, ready_q);
   	*running_pid=-1;
   }
outportb(0x20, 0x60);
//   if the runtime has reached TIME_LIMIT:
//      reset its runtime
//      change its state to READY
//      queue it to ready queue
//      set running PID to -1
//      (Scheduler() will get next PID from ready queue if any;
//      if none, Scheduler will pick 0 as running PID)
}
