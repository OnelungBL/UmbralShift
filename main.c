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

// kernel data stuff:
int running_pid;            // currently-running PID, if -1, none running
q_t ready_q, free_q;        // processes ready to run and ID's un-used
pcb_t pcb[MAX_PROC_NUM];    // process table
char proc_stack[MAX_PROC_NUM][PROC_STACK_SIZE]; // runtime stacks of processes

int main() {
   int pid;

   InitKernelData(); //call InitKernelData()  to set kernel data

   DeQ(free_q); //call DeQ() to dequeue free_q to get pid
   StartProcISR(pid); //call StartProcISR(pid) to create IdleProc

   while(1){//infinite loop to alternate 2 things below:
      LoadRun(); //call LoadRun() to load/run the chosen process
      KernelMain(); //call KernelMain() to run kernel periodically to control things
   }
   return 0;   // not reached, but compiler needs it for syntax
}

void InitKernelData() {
   int i;

   MyBZero(some char p, some int bytesize); //call MyBzero() to clear queues (which is to be coded in toolfunc.h/.c)

   for(int i=0; i<20; i++){ //loop number i from 0 to 19:
      EnQ(i, free_q); //call EnQ() to enqueue i to free_q
      MyBZero(some char p, some int bytesize); //call MyBzero() to clear pcb[i]
   }
   running_pid = 0; //set running_pid to 0;  none initially, need to chose by Scheduler()
}

void Scheduler() {  // to choose running PID
   if(running_pid>0){//simply return if running_pid is greater than 0 (0 or less/-1 continues)
	return;
   }
   if(running_pid==0){ //if running process ID is 0 (IdleProc), change its state to READY (from RUN)
   	
   }

   //set running process ID = dequeue ready_q
   if(running_pid==-1){ //if it's -1 (didn't get one, ready_q was empty)
      running_pid = 0; //set running process ID = 0 (fall back to IdleProc)
   }

   //whoever's now selected as running process, set its state to RUN
}

void KernelMain() {
   int new_pid;
   char key;

   TimerISR(); //call TimerISR() to service timer interrupt as if it just occurred

   if(cons_kbhit()){ //if a key has been pressed on PC {
      key = cons_getchar(); //read the key with cons_getchar()
      switch(key) {
         case 's'
            new_pid = DeQ(free_q); //dequeue free_q for a new pid
            if(new_pid == -1) //if the new pid (is -1) indicates no ID left
               cons_printf("Panic: no more available process ID left!\n"); //show msg on target PC: "Panic: no more available process ID left!\n"
            else
               StartProcISR(new_pid)//call StartProcISR(new pid) to create new proc
            break;

         case 'e'
            EndProcISR(); //call EndProcISR() to handle this event
            break;

         case 'b'
            breakpoint(); //call breakpoint(); to go into GDB
            break;

         case 'x'
            exit(0); //just call exit(0) to quit MyOS.dli
     }
   }
   Scheduler(); //call Scheduler() to choose next running process if needed
}

