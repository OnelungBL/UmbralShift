// isr.c, 159

#include "spede.h"
#include "typedef.h"
#include "isr.h"
#include "toolfunc.h"
#include "extern.h"
#include "proc.h"
#include "syscall.h"
#include "main.h"
#include <spede/flames.h>
#include <spede/machine/io.h>
#include <spede/machine/pic.h>

void StartProcISR(int new_pid, int func_addr) {
  pcb[new_pid].runtime = 0;
  pcb[new_pid].total_runtime = 0;
  pcb[new_pid].state = READY; //set its state to READY
  if (new_pid != 0) {
    EnQ(new_pid, &ready_q);
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
      pcb[new_pid].TF_ptr->eip = func_addr;  //will also be able to set EIP of the trapframe to func_addr (instead of hardcoding)
   }	
}

void TimerISR() {
   if(running_pid==-1){ //just return if running PID is -1 (not any valid PID)
        cons_printf("Uh oh!");  //There was a problem!
	return; //(shouldn't happen, a Panic message can be considered)
   } 
   pcb[running_pid].runtime++;
   pcb[running_pid].total_runtime++;
   if (pcb[running_pid].runtime >= TIME_LIMIT) {
   	pcb[running_pid].runtime = 0;
   	pcb[running_pid].state=READY;
   	EnQ(running_pid, &ready_q);
   	running_pid=-1;
   }
}

void GetPidISR() {
  pcb[running_pid].TF_ptr->eax = running_pid;
}

void SleepISR() {
  pcb[running_pid].wake_time = OS_clock + pcb[running_pid].TF_ptr->eax * 100;
  EnQ(running_pid, &sleep_q);
  pcb[running_pid].state = SLEEP;
  running_pid=-1;
}

void SemGetISR(int limit) {
  int sem_id = DeQ(&sem_q);
  if (sem_id == -1) {
  	pcb[running_pid].TF_ptr->ebx = -1;
  	return;
  }
 MyBzero((char *)&sem[sem_id], sizeof(sem_t));
 sem[sem_id].limit = limit;
 pcb[running_pid].TF_ptr->ebx = sem_id;
}



void SemWaitISR(int sem_id) {
  if (sem[sem_id].limit > 0) {
  	sem[sem_id].limit--;
  	return;
  }
  cons_printf("\nSemWaitISR(): blocking proc %d <---", running_pid);
  EnQ(running_pid, &sem[sem_id].wait_q);
  pcb[running_pid].state = WAIT;
  running_pid = -1;
}

void SemPostISR(int sem_id) {
  int released_pid;
  if (sem[sem_id].wait_q.len == 0) {
  	sem[sem_id].limit++;
  	return;
  }
  cons_printf("\nSemPostISR(): freeing proc %d <---", running_pid);
  released_pid = DeQ(&sem[sem_id].wait_q);
  pcb[released_pid].state = READY;
  EnQ(released_pid, &ready_q);
}

