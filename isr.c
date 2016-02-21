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
  
// build initial trapframe in proc stack,
// call MyBzero() to clear the stack 1st

   MyBzero((char *)&proc_stack[new_pid], PROC_STACK_SIZE); //call MyBzero() to clear pcb[i]


// set TF_ptr of PCB to close to end (top) of stack, then fill out

   pcb[new_pid].TF_ptr = (TF_t *)&proc_stack[new_pid][PROC_STACK_SIZE];
   pcb[new_pid].TF_ptr--; // (against last byte of stack, has space for a trapframe to build)

   pcb[new_pid].TF_ptr->eflags = EF_DEFAULT_VALUE|EF_INTR; // set INTR flag
   pcb[new_pid].TF_ptr->cs = get_cs();                     // standard fair
   pcb[new_pid].TF_ptr->ds = get_ds();                     // standard fair
   pcb[new_pid].TF_ptr->es = get_es();                     // standard fair
   pcb[new_pid].TF_ptr->fs = get_fs();                     // standard fair
   pcb[new_pid].TF_ptr->gs = get_gs();                     // standard fair

   if(new_pid == 0) {
      pcb[new_pid].TF_ptr->eip = (unsigned) IdleProc; //?...     // if pid is 0, points to IdleProc
   } else {
      pcb[new_pid].TF_ptr->eip = (unsigned) UserProc;
   }
}

void EndProcISR(int *running_pid, q_t *free_q, pcb_t *pcb) {
   if(*running_pid == 0){ //if running PID is 0 (IdleProc should not let exit),
      return; //then, just return;
   } else {
   	pcb[*running_pid].state=FREE;
   	EnQ(*running_pid, free_q);
   	*running_pid=-1;
   }
}        

void TimerISR(int *running_pid, q_t *ready_q, pcb_t *pcb) {
	outportb(0x20, 0x60);
   if(*running_pid==-1){ //just return if running PID is -1 (not any valid PID)
        cons_printf("Uh oh!");  //There was a problem!
	return;
   }//(shouldn't happen, a Panic message can be considered)
   //if(*running_pid==0) {
   //  return;
   //} //no need to swap out process zero if it's the only process running
   
   
   outportb(0x20, 0x60);
   //in PCB, upcount both runtime and total_runtime of running process
   pcb[*running_pid].runtime++;
   pcb[*running_pid].total_runtime++;
   printf("running pid: %d runtime: %d, total runtime: %d\n", *running_pid, pcb[*running_pid].runtime, pcb[*running_pid].total_runtime);
   if (pcb[*running_pid].runtime >= TIME_LIMIT) {
   	pcb[*running_pid].runtime = 0;
   	pcb[*running_pid].state=READY;
   	EnQ(*running_pid, ready_q);
   	*running_pid=-1;
   	outportb(0x20, 0x60);
   }
outportb(0x20, 0x60);
outportb(0x20, 0x60);

//int x;
  //for(x=0; x<20; x++) {
  //	printf("ready queue[%d]: %d\n", x, ready_q->q[x]);
  //}
}
