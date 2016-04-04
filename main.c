// main.c, 159
// the kernel is simulated, not yet real
//
// Team Name: UmbralShift (Members: Darren Takemoto and Alysha Straub)
// Phase 4

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
q_t ready_q, free_q, sleep_q;        // processes ready to run and ID's un-used
pcb_t pcb[MAX_PROC_NUM];    // process table
char proc_stack[MAX_PROC_NUM][PROC_STACK_SIZE]; // runtime stacks of processes
int OS_clock;
struct i386_gate *IDT_ptr;
sem_t sem[Q_SIZE];
q_t sem_q;
msg_q_t msg_q[MAX_PROC_NUM];
int printing_semaphore;

int main() {
   int pid;

   InitKernelData(); //call InitKernelData()  to set kernel data
   InitKernelControl();

  pid = DeQ(&free_q); //call DeQ() to dequeue free_q to get pid 0 (Idle)
  StartProcISR(pid, (int) IdleProc);
  pid = DeQ(&free_q); //call DeQ() to dequeue free_q to get pid 1 (Init)
  StartProcISR(pid, (int) InitProc);
  pid = DeQ(&free_q); //call DeQ() to dequeue free_q to get pid 2 (Printer)
  StartProcISR(pid, (int) PrintDriver);
  LoadRun(pcb[0].TF_ptr);

   return 0;   // not reached, but compiler needs it for syntax
}

void SetEntry(int entry_num, func_ptr_t func_ptr) {
	struct i386_gate *gateptr = &IDT_ptr[entry_num];
	fill_gate(gateptr, (int)func_ptr, get_cs(), ACC_INTR_GATE, 0);
}

void InitKernelData() {
   int i;
   OS_clock = 0; //reset os_clock to zero:  Procedure.txt line: 25
   MyBzero((char *)&sleep_q, sizeof(q_t));  //reset sleep_q: Procedure.txt line:26
   MyBzero((char *)&free_q, sizeof(q_t)); //call MyBzero() to clear queues (which is to be coded in toolfunc.h/.c)
   MyBzero((char *)&ready_q, sizeof(q_t)); //call MyBzero() to clear queues (which is to be coded in toolfunc.h/.c)
   MyBzero((char *)&sem_q, sizeof(q_t));
   for(i=0; i<Q_SIZE; i++) {
   	EnQ(i, &sem_q);
   }
   MyBzero((char *)&msg_q, sizeof(msg_q_t));

   for(i=0; i<MAX_PROC_NUM; i++){ //loop number i from 0 to 19:
      EnQ(i, &free_q); //call EnQ() to enqueue i to free_q
      MyBzero((char *)&pcb[i], sizeof(pcb_t)); //call MyBzero() to clear pcb[i]
      MyBzero((char*)&msg_q[i], sizeof(msg_t)); //clear each queue in msg queue
   }
   running_pid = 0; //set running_pid to 0;  none initially, need to chose by Scheduler()
//   product_count = 0;
}

void InitKernelControl() { // learned from timer lab, remember to modify main.h
   IDT_ptr = get_idt_base(); //locate IDT 1st
   SetEntry(TIMER_INTR, TimerEntry);	// prime IDT entry //call SetEntry() to plant TimerEntry jump point
   SetEntry(IRQ7_INTR, IRQ7Entry);
   SetEntry(GETPID_INTR, GetPidEntry);
   SetEntry(SLEEP_INTR, SleepEntry);
   SetEntry(STARTPROC_INTR, StartProcEntry);
   SetEntry(SEMGET_INTR, SemGetEntry);
   SetEntry(SEMWAIT_INTR, SemWaitEntry);
   SetEntry(SEMPOST_INTR, SemPostEntry);
   SetEntry(MSGSND_INTR, MsgSndEntry);
   SetEntry(MSGRCV_INTR, MsgRcvEntry);
   outportb(0x21, ~1);		// 0x21 is PIC mask, ~1 is mask //program the mask of PIC
   outportb(0x67, ~1); //IRQ0 and IRQ7?
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
  int new_pid;
  int idx;
  int sleepQLen;
   pcb[running_pid].TF_ptr = TF_ptr;  //save TF_ptr to PCB of running process

  switch(TF_ptr->intr_id) {
//http://athena.ecs.csus.edu/~changw/159/0/ManualCh8IRQandIO.pdf  - outport reference
  	case TIMER_INTR:
  		outportb(0x20, 0x60);
  		TimerISR(); //dismiss timer event: send PIC with a code
                OS_clock++;
		sleepQLen = sleep_q.len;
                for(idx=0; idx<sleepQLen; idx++) {
                   int pid;
                   pid = DeQ(&sleep_q);
                   if (pid == -1) {
                     running_pid = 0;
                   }
                   if (OS_clock == pcb[pid].wake_time) {
                        EnQ(pid, &ready_q);
                        pcb[pid].state = READY;
                   } else {
                        EnQ(pid, &sleep_q);
                   }
                }
  		break;
        case IRQ7_INTR:
                  outportb(0x20,0x67);
                  SemPostISR(TF_ptr->eax);
                break;
        case GETPID_INTR:
                GetPidISR();
                break;
        case SLEEP_INTR:
                SleepISR();
                break;
        case STARTPROC_INTR:
		new_pid = DeQ(&free_q);
        	StartProcISR(new_pid, TF_ptr->eax);
        	break;
        case SEMGET_INTR:
        	SemGetISR(TF_ptr->eax);
        	break;
        case SEMWAIT_INTR:
        	SemWaitISR(TF_ptr->eax);
        	break;
        case SEMPOST_INTR:
        	SemPostISR(TF_ptr->eax);
        	break;
        case MSGSND_INTR:
                MsgSndISR(TF_ptr->eax);
                break;
        case MSGRCV_INTR:
                MsgRcvISR(TF_ptr->eax);
                break;
  	default:
  		cons_printf("Panic: unknown intr ID(%d)!\n", TF_ptr->intr_id);
  		breakpoint();
  		break;
  }
Scheduler(); //call Scheduler() to chose process to load/run if needed
LoadRun(pcb[running_pid].TF_ptr); //call LoadRun(pcb[running_pid].TF_ptr) to load/run selected proc
}





