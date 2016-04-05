// syscall.c
// software interrupt/syscalls, i.e., kernel service API
#include "typedef.h"


int GetPid() {                   // no input, has return
	int pid;

	asm("int $48; movl %%eax, %0" // CPU inst
	: "=g" (pid)              // output from asm("...")
	:                         // no input into asm("...")
	: "%eax");                // will get pushed before asm("..."), and popped after

	return pid;
}

void Sleep(int seconds) {        // has input, no return

	asm("movl %0, %%eax; int $49" // CPU inst
	:                         // no output from asm("...")
	: "g" (seconds)           // input into asm("...")
	: "%eax");                // will get pushed before asm("..."), and popped after
}

void StartProc(func_ptr_t func_ptr) {
	asm("movl %0, %%eax; int $50" // CPU inst
	:                       
	: "g" ((int) func_ptr)
	: "%eax");                
}

int SemGet(int limit) {
	int semID;
	asm("movl %1, %%eax; int $51; movl %%ebx, %0"
	: "=g" (semID)
	: "g" (limit)
	: "%eax", "%ebx");
	return semID;
}

void SemWait(int semID) {
	asm("movl %0, %%eax; int $52"
	:                        
	: "g" (semID)         
	: "%eax");                
}

void SemPost(int semID) {
	asm("movl %0, %%eax; int $53"
	:                        
	: "g" (semID)         
	: "%eax"); 
}

void MsgSnd(int mid, msg_t *msg) {
	asm("movl %0, %%eax; movl %1, %%ebx; int $54"
	:                                 
	: "g" (mid), "g" (msg)
	: "%eax", "%ebx"); 
}

void MsgRcv(msg_t *msg_addr) {
	asm("movl %0, %%eax; int $55"
	:                                
	: "g" (msg_addr)        
	: "%eax"); 
}


