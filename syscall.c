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

void MsgSnd(msg_t *msg_addr) {
	asm("movl %0, %%eax; int $54"
	:                        
	: "g" ((int)msg_addr)         
	: "%eax"); 
}

void MsgRcv(msg_t *msg_addr) {
	asm("movl %0, %%eax; int $55"
	:                        
	: "g" ((int)msg_addr)         
	: "%eax"); 
}

void TipIRQ3() {
        asm("int $35");
}

int Fork(char *addr, int size) { // FileService finds these info
	int pid;
	asm("movl %1, %%eax; movl %2, %%ebx; int $56; movl %%ecx, %0"
	: "=g" (pid)
	: "g" ((int)addr), "g" ((int)size)
	: "%eax", "%ebx", "%ecx");
	return pid;
}



int Wait(int *exit_code) { // return = child PID, arg = & exit #
	int pid;
	asm("int $57; movl %%eax, %1; movl %%ebx, %0"
	: "=g" (pid), "=g" (*exit_code)
	: 
	: "%eax", "%ebx");
	return pid;
}


void Exit(int *exit_code) { // exit # (to return to parent)
	asm("movl %0, %%eax; int $58"
	:                        
	: "g" ((int)exit_code)         
	: "%eax");
}



