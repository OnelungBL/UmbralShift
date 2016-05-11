// isr.c, 159

#include "spede.h"
#include "typedef.h"
#include "isr.h"
#include "toolfunc.h"
#include "extern.h"
#include "proc.h"
#include "syscall.h"
#include "main.h"
#include "FileService.h"
#include <spede/flames.h>
#include <spede/machine/io.h>
#include <spede/machine/pic.h>

void StartProcISR(int new_pid, int func_addr) {
	pcb[new_pid].runtime = 0;
	pcb[new_pid].total_runtime = 0;
	pcb[new_pid].trans_table = OS_trans_table;
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
  int this_pid;
  msg_t *incoming_msg_ptr;
  msg_t *destination;
  int msg_q_id;
  int freed_pid;
  this_pid = running_pid;
  
  set_cr3(pcb[running_pid].trans_table);
  incoming_msg_ptr = (msg_t *) msg_addr;
  set_cr3(pcb[this_pid].trans_table);
  
  msg_q_id = incoming_msg_ptr->recipient;
  incoming_msg_ptr->OS_clock = OS_clock;
  incoming_msg_ptr->sender = running_pid;
if (msg_q[msg_q_id].wait_q.len == 0) {
    MsgEnQ(incoming_msg_ptr, &msg_q[msg_q_id]);
  } else {
    freed_pid = DeQ(&msg_q[msg_q_id].wait_q);
    EnQ(freed_pid, &ready_q);
    pcb[freed_pid].state = READY;
	
	set_cr3(pcb[running_pid].trans_table);
    destination = (msg_t *)pcb[freed_pid].TF_ptr->eax;
    *destination = *incoming_msg_ptr;
	set_cr3(pcb[this_pid].trans_table);
  }
}

void MsgRcvISR(int msg_addr) {
  int this_pid;
  msg_t *receiving_msg_ptr;
  msg_t *queued_msg_ptr;
  int msg_q_id;
  this_pid = running_pid;
  receiving_msg_ptr = (msg_t *)msg_addr;
  msg_q_id = running_pid;
  if (msg_q[msg_q_id].len > 0) {
    set_cr3(pcb[running_pid].trans_table);
    queued_msg_ptr = MsgDeQ(&msg_q[msg_q_id]);
    *receiving_msg_ptr = *queued_msg_ptr;
   set_cr3(pcb[this_pid].trans_table);
	
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

void ForkISR(int data, int size) { //does addr really go here?
        int i;
        int freed_pid;
        int DRAM_page;
		int five_idx[5];
		TF_t *TF_ptr;
		int *entry_ptr;
		int main_table;
		int code_data_subtable;
		int stack_subtable;
		int code_data_page;
		int stack_page;
		
        DRAM_page = -1;

        if (free_q.len == 0) {
                cons_printf("Panic: no free PID left!\n");
                pcb[running_pid].TF_ptr->ecx = -1;
        } else {
				//get five DRAM page indices
				i=0;
				for (DRAM_page=0; DRAM_page<DRAM_PAGE_COUNT; DRAM_page++) {
					if (DRAM[DRAM_page].owner == -1) {
						five_idx[i++] = DRAM_page;
						if (i==5) break;
					}
				}
				if (DRAM_page >= DRAM_PAGE_COUNT) {
					cons_printf("Panic: no free DRAM space left!\n");
                    pcb[running_pid].TF_ptr->ecx = -1; //syscall uses ecx as return child PID
				} else {
						main_table = (int)&DRAM[five_idx[0]].addr;
						code_data_subtable = (int)&DRAM[five_idx[1]].addr;
						stack_subtable = (int)&DRAM[five_idx[2]].addr;
						code_data_page = (int)&DRAM[five_idx[3]].addr;
						stack_page = (int)&DRAM[five_idx[4]].addr;
				
				
                        freed_pid = DeQ(&free_q);
                        pcb[running_pid].TF_ptr->ecx = freed_pid;
 //                       DRAM[DRAM_page].owner = freed_pid;
						for (i=0; i<5; i++) {
							MyBzero((char *) &DRAM[five_idx[i]], sizeof(DRAM_t));
							DRAM[five_idx[i]].owner = freed_pid;
						}
						MyBzero((char *)&pcb[freed_pid], sizeof(pcb_t)); 
                        MyBzero((char *)&msg_q[freed_pid], sizeof(msg_q_t)); //call MyBzero() to clear msg_q of freed_pid
						EnQ(freed_pid, &ready_q);
                        pcb[freed_pid].state = READY;
                        pcb[freed_pid].ppid = running_pid;
						pcb[freed_pid].trans_table = main_table;
						pcb[freed_pid].TF_ptr = (TF_t *)0xbfffffc0; //3G-64
						
						MyMemcpy((char * )&code_data_page, (char *)data, size);
						
						TF_ptr = (TF_t *)(stack_page + 4096 - 64);

	                pcb[freed_pid].TF_ptr->eflags = EF_DEFAULT_VALUE|EF_INTR; // set INTR flag
	                pcb[freed_pid].TF_ptr->cs = get_cs();
	                pcb[freed_pid].TF_ptr->ds = get_ds();
	                pcb[freed_pid].TF_ptr->es = get_es();
	                pcb[freed_pid].TF_ptr->fs = get_fs();
	                pcb[freed_pid].TF_ptr->gs = get_gs();
					pcb[freed_pid].TF_ptr->eip = (int)(TF_ptr + 0x80); //DRAM[DRAM_page].addr + 0x80;
					
					//translation table, 0x400 is 1kb
					entry_ptr = (int *)main_table;
					MyMemcpy((char *)entry_ptr, (char *)OS_trans_table, (sizeof(entry_ptr) * 4)); //each entry is 1kb, copy first four entries
					
					entry_ptr = (int *)main_table;
					entry_ptr[512] = (code_data_subtable|0x30); //xor 1100000
					entry_ptr[767] = (stack_subtable|0x30);
					entry_ptr = (int *)code_data_subtable;
					entry_ptr[0] = (code_data_page|0x30);
					entry_ptr = (int *)stack_subtable;
					entry_ptr[767] = (stack_page|0x30);
                }
        }
        return;
}

void WaitISR() {
        int i;
        int dram_page;
        dram_page = -1;    
        for (i=0; i<DRAM_PAGE_COUNT; i++) {
                if ((pcb[DRAM[i].owner].ppid == running_pid) && (pcb[DRAM[i].owner].state == ZOMBIE)) {
                        dram_page=i;
                        break;
                }
        }
        
        if (dram_page == -1) {
                pcb[running_pid].state = FORKWAIT;
                running_pid = -1;
                return;
        } else {
                pcb[running_pid].TF_ptr->ebx = DRAM[dram_page].owner;
                pcb[running_pid].TF_ptr->eax = pcb[DRAM[dram_page].owner].TF_ptr->eax;
        }

        for (i=0; i<DRAM_PAGE_COUNT; i++) {
                if (DRAM[i].owner == DRAM[dram_page].owner) {
                        MyBzero((char *)&DRAM[i], sizeof(DRAM_t));
                        DRAM[i].owner=-1;
                        EnQ(DRAM[dram_page].owner, &free_q);
                        MyBzero((char*)&pcb[DRAM[dram_page].owner], sizeof(pcb_t));
                        break;
                }
        }
        return;
}
void ExitISR() {
        int i;
        int parent_pid;
        int dram_page;
        dram_page = -1;
        parent_pid = pcb[running_pid].ppid;
        if (pcb[parent_pid].state != FORKWAIT) {
                pcb[running_pid].state = ZOMBIE;
                running_pid = -1;
        } else {
                pcb[parent_pid].state = READY;
                EnQ(parent_pid, &ready_q);
                pcb[parent_pid].TF_ptr->ebx = running_pid;
                pcb[parent_pid].TF_ptr->eax = pcb[running_pid].TF_ptr->eax;
                for (i=0; i<DRAM_PAGE_COUNT; i++) {
                        if (DRAM[i].owner == running_pid) {
                                dram_page = i;
                                break;
                        }
                }
                MyBzero((char*)DRAM[dram_page].addr, sizeof(int));
                DRAM[dram_page].owner = -1;
                MyBzero((char*)&pcb[running_pid], sizeof(pcb_t));
                EnQ(running_pid, &free_q);
                running_pid = -1;
        }
        return;
}
