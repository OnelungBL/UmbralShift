// isr.c, 159

#include "spede.h"
#include "typedef.h"
#include "isr.h"
#include "toolfunc.h"
#include "extern.h"
#include "proc.h"
#include "syscall.h"
#include "main.h"
#include <spede/flames.h>
#include <spede/machine/io.h>
#include <spede/machine/pic.h>

void StartProcISR(int new_pid, int func_addr) {
	pcb[new_pid].runtime = 0;
	pcb[new_pid].total_runtime = 0;
	pcb[new_pid].state = READY; //set its state to READY
	if (new_pid != 0) {
		EnQ(new_pid, &ready_q);
	}

	MyBzero((char *)&proc_stack[new_pid], PROC_STACK_SIZE); //call MyBzero() to clear pcb[i]
	MyBzero((char *)&msg_q[new_pid], sizeof(msg_q_t)); //call MyBzero() to clear msg_q_t


	// set TF_ptr of PCB to close to end (top) of stack, then fill out

	pcb[new_pid].TF_ptr = (TF_t *)&proc_stack[new_pid][PROC_STACK_SIZE];
	pcb[new_pid].TF_ptr--; // (against last byte of stack, has space for a trapframe to build)

	pcb[new_pid].TF_ptr->eflags = EF_DEFAULT_VALUE|EF_INTR; // set INTR flag
	pcb[new_pid].TF_ptr->cs = get_cs();
	pcb[new_pid].TF_ptr->ds = get_ds();
	pcb[new_pid].TF_ptr->es = get_es();
	pcb[new_pid].TF_ptr->fs = get_fs();
	pcb[new_pid].TF_ptr->gs = get_gs();

	pcb[new_pid].TF_ptr->eip = func_addr;
}

void TimerISR() {
	if(running_pid<1){
		return;
	} 
	pcb[running_pid].runtime++;
	pcb[running_pid].total_runtime++;
	if (pcb[running_pid].runtime >= TIME_LIMIT) {
		pcb[running_pid].runtime = 0;
		pcb[running_pid].state=READY;
		EnQ(running_pid, &ready_q);
		running_pid=-1;
	}
}

void GetPidISR() {
	pcb[running_pid].TF_ptr->eax = running_pid;
}

void SleepISR() {
	pcb[running_pid].wake_time = OS_clock + pcb[running_pid].TF_ptr->eax * 100;
	EnQ(running_pid, &sleep_q);
	pcb[running_pid].state = SLEEP;
	running_pid=-1;
}

void SemGetISR(int limit) {
	int sem_id = DeQ(&sem_q);
	if (sem_id == -1) {
		pcb[running_pid].TF_ptr->ebx = -1;
		return;
	}
	MyBzero((char *)&sem[sem_id], sizeof(sem_t));
	sem[sem_id].limit = limit;
	pcb[running_pid].TF_ptr->ebx = sem_id;
}

void SemWaitISR(int sem_id) {
	if (sem[sem_id].limit > 0) {
		sem[sem_id].limit--;
		return;
	}
	EnQ(running_pid, &sem[sem_id].wait_q);
	pcb[running_pid].state = WAIT;
	running_pid = -1;
}

void SemPostISR(int sem_id) {
	int released_pid;
	if (sem[sem_id].wait_q.len == 0) {
		sem[sem_id].limit++;
		return;
	}
	released_pid = DeQ(&sem[sem_id].wait_q);
	pcb[released_pid].state = READY;
	EnQ(released_pid, &ready_q);
}


void MsgSndISR(int msg_addr) {
  msg_t *incoming_msg_ptr;
  msg_t *destination;
  int msg_q_id;
  int freed_pid;
  incoming_msg_ptr = (msg_t *) msg_addr;
  msg_q_id = incoming_msg_ptr->recipient;
  incoming_msg_ptr->OS_clock = OS_clock; //authentication ?
  incoming_msg_ptr->sender = running_pid; //authentication ?

if (msg_q[msg_q_id].wait_q.len == 0) {
    MsgEnQ(incoming_msg_ptr, &msg_q[msg_q_id]);
  } else {
    freed_pid = DeQ(&msg_q[msg_q_id].wait_q);
    EnQ(freed_pid, &ready_q);
    pcb[freed_pid].state = READY;
    destination = (msg_t *)pcb[freed_pid].TF_ptr->eax;
    *destination = *incoming_msg_ptr;
  }
}

void MsgRcvISR(int msg_addr) {
  msg_t *receiving_msg_ptr;
  msg_t *queued_msg_ptr;
  int msg_q_id;
  receiving_msg_ptr = (msg_t *)msg_addr;
  msg_q_id = running_pid; //receiving_msg_ptr->recipient;
  if (msg_q[msg_q_id].len > 0) {
    queued_msg_ptr = MsgDeQ(&msg_q[msg_q_id]);
    *receiving_msg_ptr = *queued_msg_ptr;
  } else {  //no message   
    EnQ(running_pid, &msg_q[msg_q_id].wait_q);
    pcb[running_pid].state=WAIT;
    running_pid = -1;
  } 
}

void IRQ3ISR() {
        switch(inportb(COM2_IOBASE+IIR)) {
                //case IIR_TXRDY, call TX() and break (send char to terminal)
                case IIR_TXRDY:
                        TX();
                        break;
                //case IIR_RXRDY, call RX() and break (get char from terminal)
                case IIR_RXRDY:
                        RX();
                        break;
                default:
                        break;
        }

        //if TXRDY in port data equals to 1
        if (port_data.TXRDY==1) {
                //call TX() (check if can use it now to transmit out a char)
                TX();
        }
}

void TX() { // dequeue out_q to write to port
        char ch;
        ch = 0;
        if (port_data.echo_buffer.len > 0) {
                ch = DeQ(&port_data.echo_buffer);
        } else {
                if (port_data.TX_buffer.len > 0) {
                        ch = DeQ(&port_data.TX_buffer);
                        SemPostISR(port_data.TX_semaphore);
                }
        }
        if (ch != 0) {
                //use outportb to write ch to COM2_IOBASE+DATA
                outportb(COM2_IOBASE+DATA, ch);
                //clear TXRDY of port data
                port_data.TXRDY=0;
        } else {
                //set TXRDY of port data to 1
                port_data.TXRDY=1;
        }
}

void RX() { // read char from port to queue in_q and echo_q
        char ch;
        // use 127 to mask out msb (rest 7 bits in ASCII range)
        ch = inportb(COM2_IOBASE+DATA) & 0x7F;  // mask 0111 1111

        //enqueue ch to RX buffer of port data
        EnQ(ch, &port_data.RX_buffer);
        //SemPostISR RX_semaphore of port data
        SemPostISR(port_data.RX_semaphore);

        if (ch == '\r') {
                //enqueue '\r' then '\n' to echo buffer of port data
                EnQ('\r', &port_data.echo_buffer);
                EnQ('\n', &port_data.echo_buffer);
        } else {
                if (port_data.echo_mode == 1) {
                        //enqueue ch to echo buffer of port data
                        EnQ(ch, &port_data.echo_buffer);
                }
        }
}
