// typedef.h, 159

#ifndef _TYPEDEF_H_
#define _TYPEDEF_H_

#include "TF.h"              // trapframe type TF_t is defined here

#define TIME_LIMIT 10       // max timer count to run
#define MAX_PROC_NUM 20      // max number of processes
#define Q_LEN 20             // queuing capacity
#define Q_SIZE 20
#define PROC_STACK_SIZE 4096 // process runtime stack in bytes
#define INITIAL_SEMAPHORE_LIMIT 1

// this is the same as constants defines: UNUSED=0, READY=1, etc.
typedef enum {FREE, READY, RUN, SLEEP, WAIT, ZOMBIE, FORKWAIT} state_t;

typedef struct {             // PCB describes proc image
   state_t state;            // state of process
   int runtime;              // runtime since loaded
   int total_runtime;        // total runtime since created
   TF_t *TF_ptr;             // points to trapframe of process
   int wake_time;
} pcb_t;

typedef struct {             // proc queue type
   int head, len;      // where head and tail are, and current length
   int q[Q_LEN];             // indices into q[] array to place or get element
} q_t;

typedef void (*func_ptr_t)(void); // void-return function pointer type
/*
typedef struct {
   int limit; //count to limit number of processes to access a critical code section
   q_t wait_q; //queue for blocking/waiting PIDs
} sem_t;
*/
typedef struct {
  int sender;
  int recipient;
  int OS_clock;
  int data;
} msg_t;

typedef struct {
  int head;
  int tail;
  int len;
  msg_t msg[Q_LEN];
  q_t wait_q;
} msg_q_t;

#endif
