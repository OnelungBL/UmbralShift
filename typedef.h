// typedef.h, 159

#ifndef _TYPEDEF_H_
#define _TYPEDEF_H_

#include "TF.h"              // trapframe type TF_t is defined here

#define TIME_LIMIT 10       // max timer count to run
#define MAX_PROC_NUM 20      // max number of processes
#define Q_LEN 20             // queuing capacity
#define PROC_STACK_SIZE 4096 // process runtime stack in bytes
#define INITIAL_SEMAPHORE_LIMIT 1
#define INITIAL_PRINT_SEMAPHORE_LIMIT 0
#define INITIAL_RX_SEMAPHORE_LIMIT 0
#define MSG_DATA_LENGTH 101


#define PROC_IDLE 0
#define PROC_INIT 1
#define PROC_PRINT 2
#define PROC_SHELL 3
#define PROC_STDIN 4
#define PROC_STDOUT 5

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
	int head, tail, len;      // where head and tail are, and current length
	int q[Q_LEN];             // indices into q[] array to place or get element
} q_t;

typedef void (*func_ptr_t)(void); // void-return function pointer type

typedef struct {
	int limit; //count to limit number of processes to access a critical code section
	q_t wait_q; //queue for blocking/waiting PIDs
} sem_t;

typedef struct {
	int sender;
	int recipient;
	int OS_clock;
	char data[MSG_DATA_LENGTH];
} msg_t;

typedef struct {
	int head;
	int tail;
	int len;
	msg_t msg[Q_LEN];
	q_t wait_q;
} msg_q_t;

typedef struct {
	q_t TX_buffer;      // transmit to terminal
	q_t RX_buffer;      // receive from terminal
	q_t echo_buffer;    // echo back to terminal
	int TX_semaphore;   // transmit space count
	int RX_semaphore;   // receive data count
	int echo_mode;      // if 1, echo back to terminal
	int TXRDY;          // if 1, TXRDY event occurred
	int stdin_pid;      // StdinProc PID (who's upper half)
	int stdout_pid;     // StdoutProc PID (who's upper half)
} port_data_t;


#endif
