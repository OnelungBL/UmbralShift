// isr.c, 159

#include "spede.h"
#include "type.h"
#include "isr.h"
#include "tool.h"
#include "extern.h"
#include "proc.h"

void StartProcISR(int new_pid) {
   clear the PCB of the new pid
   set its state to READY

   if new pid is not 0 (IdleProc),
      then, enqueue this new pid into the ready queue
}

void EndProcISR() {
   id(running_pid == 0){ //if running PID is 0 (IdleProc should not let exit),
      return; //then, just return;
   }
   change state of running process to FREE
   queue the running PID to free queue
   set running PID to -1 (now none)
}        

void TimerISR() {
   if(running_pid==-1){ //just return if running PID is -1 (not any valid PID)
	return;
   }
   //(shouldn't happen, a Panic message can be considered)
   
   outportb(0x20, 0x60);
   in PCB, upcount both runtime and total_runtime of running process

   if the runtime has reached TIME_LIMIT:
      reset its runtime
      change its state to READY
      queue it to ready queue
      set running PID to -1
      (Scheduler() will get next PID from ready queue if any;
      if none, Scheduler will pick 0 as running PID)
}
