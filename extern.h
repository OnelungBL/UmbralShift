// extern.h, 159

#ifndef _EXTERN_H_
#define _EXTERN_H_

#include "typedef.h"                // defines q_t, pcb_t, MAX_PROC_NUM, PROC_STACK_SIZE

extern int running_pid;             // PID of currently-running process, -1 means none
extern q_t ready_q, free_q;                        // ready to run, not used proc IDs
extern pcb_t pcb[MAX_PROC_NUM];                    // process table
extern char proc_stack[MAX_PROC_NUM][PROC_STACK_SIZE]; // process runtime stacks

#endif
