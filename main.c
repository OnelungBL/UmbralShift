// main.c, 159
// the kernel is simulated, not yet real
//
// Team Name: UmbralShift (Members: Darren Takemoto and Alysha Straub)

#include "spede.h"      // spede stuff
#include "main.h"       // main stuff
#include "isr.h"        // ISR's
#include "toolfunc.h"   // handy functions for Kernel
#include "proc.h"       // processes such as IdleProc()
#include "typedef.h"    // data types
#include "entry.h"	// TIMER_INTR/TimerEntry
#include "extern.h"

// kernel data stuff:
 int running_pid;            // currently-running PID, if -1, none running
 q_t ready_q, free_q;        // processes ready to run and ID's un-used
 pcb_t pcb[MAX_PROC_NUM];    // process table
 char proc_stack[MAX_PROC_NUM][PROC_STACK_SIZE]; // runtime stacks of processes
struct i386_gate *IDT_ptr;

int main() {
   int pid;

   InitKernelData(); //call InitKernelData()  to set kernel data
   InitKernelControl();

   pid = DeQ(&free_q); //call DeQ() to dequeue free_q to get pid
   StartProcISR(pid, &ready_q, pcb); //call StartProcISR(pid) to create IdleProc

   while(1){//infinite loop to alternate 2 things below:
      LoadRun(running_pid); //call LoadRun() to load/run the chosen process
      KernelMain(); //call KernelMain() to run kernel periodically to control things
   }
   return 0;   // not reached, but compiler needs it for syntax
}

void SetEntry(int entry_num, func_ptr_t func_ptr) {
	struct i386_gate *gateptr = &IDT_ptr[entry_num];
	fill_gate(gateptr, (int)func_ptr, get_cs(), ACC_INTR_GATE, 0);
}

void InitKernelData() {
   int i;

   MyBzero((char *)&free_q, sizeof(free_q)); //call MyBzero() to clear queues (which is to be coded in toolfunc.h/.c)
   MyBzero((char *)&ready_q, sizeof(ready_q)); //call MyBzero() to clear queues (which is to be coded in toolfunc.h/.c)
   
//   free_q.head=0;
//   free_q.tail=0;
//   free_q.len=0;
//   ready_q.head=0;
//   ready_q.tail=0;
//   ready_q.len=0;
   

   for(i=0; i<20; i++){ //loop number i from 0 to 19:
      EnQ(i, &free_q); //call EnQ() to enqueue i to free_q
      MyBzero((char *)&pcb[i], sizeof(pcb[i])); //call MyBzero() to clear pcb[i]
   }
   running_pid = 0; //set running_pid to 0;  none initially, need to chose by Scheduler()
}

void InitKernelControl() { // learned from timer lab, remember to modify main.h
   IDT_ptr = get_idt_base(); //locate IDT 1st
   SetEntry(32, TimerEntry);	// prime IDT entry //call SetEntry() to plant TimerEntry jump point
   outportb(0x21, ~1);		// 0x21 is PIC mask, ~1 is mask //program the mask of PIC
   //(but NO "sti" which is built into the process trapframe)
}

void Scheduler() {  // to choose running PID
   if(running_pid>0){//simply return if running_pid is greater than 0 (0 or less/-1 continues)
	return;
   }
   if(running_pid==0){ //if running process ID is 0 (IdleProc), change its state to READY (from RUN)
   	pcb[running_pid].state=READY;
   }

   running_pid=DeQ(&ready_q); //set running process ID = dequeue ready_q
   if(running_pid==-1){ //if it's -1 (didn't get one, ready_q was empty)
      running_pid = 0; //set running process ID = 0 (fall back to IdleProc)
   }
   //whoever's now selected as running process, set its state to RUN
   pcb[running_pid].state=RUN;
}

void KernelMain(TF_t *TF_ptr) {
  char key;
  int new_pid;
   pcb[running_pid].TF_ptr = TF_ptr;  //save TF_ptr to PCB of running process

  switch(TF_ptr->intr_id) {
  	case TIMER_INTR:
  		TimerISR(&running_pid, &ready_q, pcb);
// ------         dismiss timer event: send PIC with a code
  		break;
  	default:
  		cons_printf("Panic: unknown intr ID(%d)!\n", TF_ptr->intr_id);
  		breakpoint();
  }
//same as in Simulated:
//poll key and handle keystroke simulated events (s/e/b/q, but no 't' key)
  if(cons_kbhit()){ //if a key has been pressed on PC {
     key = cons_getchar(); //read the key with cons_getchar()
     switch(key) {
        case 's':
           new_pid = DeQ(&free_q); //dequeue free_q for a new pid
           if(new_pid == -1) { //if the new pid (is -1) indicates no ID left
              cons_printf("Panic: no more available process ID left!\n"); //show msg on target PC: "Panic: no more available process ID left!\n"
           } else {
              StartProcISR(new_pid, &ready_q, pcb);  //call StartProcISR(new pid) to create new proc
           }
           break;
        case 'e':
           EndProcISR(&running_pid, &free_q, pcb); //call EndProcISR() to handle this event
           break;
        case 'b':
           breakpoint(); //call breakpoint(); to go into GDB
           break;
        case 'x':
           exit(0); //just call exit(0) to quit MyOS.dli
    }
 }
 //-----------Scheduler(); //call Scheduler() to choose next running process if needed
//-------------   call Scheduler() to chose process to load/run if needed
//------------   call LoadRun(pcb[running_pid].TF_ptr) to load/run selected proc
}


//void KernelMain() {
//   int new_pid;
//   char key;
//
//   TimerISR(&running_pid, &ready_q, pcb); //call TimerISR() to service timer interrupt as if it just occurred
//
//   if(cons_kbhit()){ //if a key has been pressed on PC {
//      key = cons_getchar(); //read the key with cons_getchar()
//      switch(key) {
//         case 's':
//            new_pid = DeQ(&free_q); //dequeue free_q for a new pid
//            if(new_pid == -1) { //if the new pid (is -1) indicates no ID left
//               cons_printf("Panic: no more available process ID left!\n"); //show msg on target PC: "Panic: no more available process ID left!\n"
//            } else {
//               StartProcISR(new_pid, &ready_q, pcb);  //call StartProcISR(new pid) to create new proc
//            }
//            break;
//         case 'e':
//            EndProcISR(&running_pid, &free_q, pcb); //call EndProcISR() to handle this event
//            break;
//         case 'b':
//            breakpoint(); //call breakpoint(); to go into GDB
//            break;
//         case 'x':
//            exit(0); //just call exit(0) to quit MyOS.dli
//     }
//  }
//  Scheduler(); //call Scheduler() to choose next running process if needed
//}

