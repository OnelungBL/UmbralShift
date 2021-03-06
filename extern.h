// extern.h, 159

#ifndef _EXTERN_H_
#define _EXTERN_H_

#include "typedef.h"

extern int running_pid;             // PID of currently-running process, -1 means none
extern q_t ready_q, free_q, sleep_q;                        // ready to run, not used proc IDs
extern pcb_t pcb[MAX_PROC_NUM];                    // process table
extern char proc_stack[MAX_PROC_NUM][PROC_STACK_SIZE]; // process runtime stacks
extern int OS_clock;
extern sem_t sem[Q_LEN]; //semaphore array
extern q_t sem_q; //semaphore ID queue
extern msg_q_t msg_q[MAX_PROC_NUM];
extern int printing_semaphore;
extern port_data_t port_data;
extern DRAM_t DRAM[DRAM_PAGE_COUNT];
extern int OS_trans_table;
extern TF_t pf_tf_ptr;
extern int pf_err_num;
extern int pf_err_addr;
extern int pf_err_pid;
#endif
