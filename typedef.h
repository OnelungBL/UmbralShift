// typedef.h, 159

#ifndef _TYPEDEF_H_
#define _TYPEDEF_H_

#define TIME_LIMIT 3         // max timer count to run
#define MAX_PROC_NUM 20      // max number of processes
#define Q_LEN 20             // queuing capacity
#define PROC_STACK_SIZE 4096 // process runtime stack in bytes

// this is the same as constants defines: UNUSED=0, READY=1, etc.
typedef enum {FREE, READY, RUN, SLEEP, WAIT, ZOMBIE, FORKWAIT} state_t;

typedef struct {             // PCB describes proc image
   state_t state;            // state of process
   int runtime;              // runtime since loaded
   int total_runtime;        // total runtime since created
} pcb_t;

typedef struct {             // proc queue type
   int head, tail, len;      // where head and tail are, and current length
   int q[Q_LEN];             // indices into q[] array to place or get element
} q_t;

#endif
