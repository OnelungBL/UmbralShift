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
  msg_t *incoming_msg_ptr;
  msg_t *destination;
  int msg_q_id;
  int freed_pid;

  incoming_msg_ptr = (msg_t *) msg_addr;
  msg_q_id = incoming_msg_ptr->recipient;
  incoming_msg_ptr->OS_clock = OS_clock;
  incoming_msg_ptr->sender = running_pid;
if (msg_q[msg_q_id].wait_q.len == 0) {
    MsgEnQ(incoming_msg_ptr, &msg_q[msg_q_id]);
  } else {
    freed_pid = DeQ(&msg_q[msg_q_id].wait_q);
    EnQ(freed_pid, &ready_q);
    pcb[freed_pid].state = READY;
  set_cr3(pcb[freed_pid].trans_table);
    destination = (msg_t *)pcb[freed_pid].TF_ptr->eax;
    *destination = *incoming_msg_ptr;

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
    queued_msg_ptr = MsgDeQ(&msg_q[msg_q_id]);
    *receiving_msg_ptr = *queued_msg_ptr;
  } else {  //no message   
    EnQ(running_pid, &msg_q[msg_q_id].wait_q);
    pcb[running_pid].state=WAIT;
set_cr3(OS_trans_table);
    running_pid = -1;
  } 
}

void IRQ3ISR() {
        switch(inportb(COM2_IOBASE+IIR)) {
                case IIR_TXRDY:
                        TX();
                        break;
                case IIR_RXRDY:
                        RX();
                        break;
                default:
                        break;
        }

        if (port_data.TXRDY==1) {
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
                outportb(COM2_IOBASE+DATA, ch);
                port_data.TXRDY=0;
        } else {
                port_data.TXRDY=1;
        }
}

void RX() { // read char from port to queue in_q and echo_q
        char ch;
        ch = inportb(COM2_IOBASE+DATA) & 0x7F;  // mask 0111 1111
        EnQ(ch, &port_data.RX_buffer);
        SemPostISR(port_data.RX_semaphore);

        if (ch == '\r') {
                EnQ('\r', &port_data.echo_buffer);
                EnQ('\n', &port_data.echo_buffer);
        } else {
                if (port_data.echo_mode == 1) {
                        EnQ(ch, &port_data.echo_buffer);
                }
        }
}


void ForkISR(int data, int size) {
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
						main_table = DRAM[five_idx[0]].addr;
						code_data_subtable = DRAM[five_idx[1]].addr;
						stack_subtable = DRAM[five_idx[2]].addr;
						code_data_page = DRAM[five_idx[3]].addr;
						stack_page = DRAM[five_idx[4]].addr;
                        freed_pid = DeQ(&free_q);
                        pcb[running_pid].TF_ptr->ecx = freed_pid;
						for (i=0; i<5; i++) {
							DRAM[five_idx[i]].owner = freed_pid;
						}
						MyBzero((char *)&pcb[freed_pid], sizeof(pcb_t)); 
                        MyBzero((char *)&msg_q[freed_pid], sizeof(msg_q_t)); //call MyBzero() to clear msg_q of freed_pid
						EnQ(freed_pid, &ready_q);
                        pcb[freed_pid].state = READY;
                        pcb[freed_pid].ppid = running_pid;
						pcb[freed_pid].trans_table = main_table;
						pcb[freed_pid].TF_ptr = (TF_t *)0xbfffffbc; //3G-68~ 0xbfffffc0; //3G-64 //virtual?
						MyMemcpy((char * )code_data_page, (char *)data, size);
						TF_ptr = (TF_t *)(stack_page + 0x1000 - sizeof(TF_t)); //use "top" of stack_page to setup trap frame.  //real?
						TF_ptr->eip = (0x80000000 + 0x80); //eip set to beginning of runtime space, plus header
						TF_ptr->eflags = EF_DEFAULT_VALUE|EF_INTR; // set INTR flag
						TF_ptr->cs = get_cs();
						TF_ptr->ds = get_ds();
						TF_ptr->es = get_es();
						TF_ptr->fs = get_fs();
						TF_ptr->gs = get_gs();
					entry_ptr = (int *)main_table;
					MyMemcpy((char *)entry_ptr, (char *)OS_trans_table, (0x400 * 4)); //each entry is 1kb, copy first four entries
					entry_ptr += 512;
					*entry_ptr = code_data_subtable;
					*entry_ptr |= 0x3; 
					entry_ptr += 255; //767
					*entry_ptr = stack_subtable;
					*entry_ptr |= 0x3;
					entry_ptr = (int *)code_data_subtable;
					entry_ptr += 0;
					*entry_ptr = code_data_page;
					*entry_ptr |= 0x3;
					entry_ptr = (int *)stack_subtable; //1023
					entry_ptr  += 1023;
					*entry_ptr = stack_page;
					*entry_ptr |= 0x3;
                }
        }
        return;
}


void WaitISR() {
        int i;
        int dram_page;
		int page_count;
		int tmp;
        dram_page = -1;
		page_count = 0;    
        for (i=0; i<DRAM_PAGE_COUNT; i++) {
                if ((pcb[DRAM[i].owner].ppid == running_pid) && (pcb[DRAM[i].owner].state == ZOMBIE)) {
                        dram_page=i;  //at least on zombie child, returns the first zombie page found
                        break;
                }
        }
        
        if (dram_page == -1) {
                pcb[running_pid].state = FORKWAIT;
				set_cr3(OS_trans_table);
                running_pid = -1;
                return;
        } else {
                pcb[running_pid].TF_ptr->ebx = DRAM[dram_page].owner;
                pcb[running_pid].TF_ptr->eax = pcb[DRAM[dram_page].owner].TF_ptr->eax;
        }

		tmp = DRAM[dram_page].owner;
        for (i=0; i<DRAM_PAGE_COUNT; i++) {
                if (DRAM[i].owner == DRAM[dram_page].owner) {
                        DRAM[i].owner=-1;
                        MyBzero((char*)&pcb[DRAM[dram_page].owner], sizeof(pcb_t));
                        page_count++;
						if (page_count >= 5) break;
                }
        }
		EnQ(tmp, &free_q);
        return;
}

void ExitISR() {
        int i;
        int parent_pid;
        int dram_page;

        dram_page = 0;
        parent_pid = pcb[running_pid].ppid;
        if (pcb[parent_pid].state != FORKWAIT) {
                pcb[running_pid].state = ZOMBIE;
				set_cr3(OS_trans_table);
                running_pid = -1;
        } else {
                pcb[parent_pid].state = READY;
                EnQ(parent_pid, &ready_q);
                pcb[parent_pid].TF_ptr->ebx = running_pid;
                pcb[parent_pid].TF_ptr->eax = pcb[running_pid].TF_ptr->eax;
                for (i=0; i<DRAM_PAGE_COUNT; i++) {
                        if (DRAM[i].owner == running_pid) {
							DRAM[i].owner = -1;
							dram_page++;
							if (dram_page==5) break;
                        }
                }
                MyBzero((char*)&pcb[running_pid], sizeof(pcb_t));
                EnQ(running_pid, &free_q);
				set_cr3(OS_trans_table);
                running_pid = -1;
        }
        return;
}

//when a page fault occurs, an error code is placed on the top of the stack
//cr2 contains virtual address that caused the page fault

/*This does not work as well as it should
 *We got the page fault to pick up and handle page faults... with some manual
 *It appears that there is a bug with the location of the code_data_subtable when 
 *retreiving it in PF_ISR (though it seems to be storing in ForkISR okay).
 *The program also glitches at the end as another page fault is fired from source
 *not yet determined.
*/
void PF_ISR() {
	unsigned long error_addr;
	int error_number;
	int error_pid;
	int main_table_idx;
//	int *main_table;
	int sub_table_idx;
	int *sub_table;
//	int page_offset;
//	int *entry_ptr;
	char error_prompt[MSG_DATA_LENGTH];
	int i;

	error_pid = running_pid;
	error_addr = get_cr2();
    error_number = pcb[error_pid].TF_ptr->err_num;

    // error code:   6       5           4            3       2      1       0
	// [stuff]  [Dirty ][Accessed] [Cache Use][Cache Policy][User][Write][Present]
	sprintf(error_prompt, "PID: %d Err: (%u) encountered a page fault at: %lx (%lu)", error_pid, error_number, error_addr, error_addr);
	cons_printf("\n%s\n", error_prompt);

    if ((error_number & ( 1 << 6 )) >> 6) {/*printf("    (Dirty flag -> content changed)\n");*/}else{/*printf("    (Dirty flag -> content not changed)\n");*/}
	if ((error_number & ( 1 << 5 )) >> 5) {/*printf("    (Accessed flag -> accessed)\n");*/}else{/*printf("    (Accessed flag -> not looked up by MMU)\n");*/}
	if ((error_number & ( 1 << 4 )) >> 4) {/*printf("    (Cache Use flag -> to be cached) ");*/}else{/*printf("    (Cache Use flag -> not to be cached)\n");*/}
	if ((error_number & ( 1 << 3 )) >> 3) {/*printf("    (Cache Policy flag -> write through)\n");*/}else{/*printf("  (Cache Policy flag -> write back)\n");*/}
	if ((error_number & ( 1 << 2 )) >> 2) {/*printf("    (User flag -> user can access)\n");*/}else{/*printf("    (User flag -> superuser to access)\n");*/}
	if ((error_number & ( 1 << 1 )) >> 1) {/*printf("    (Write flag -> read+writable)\n");*/}else{cons_printf("    (Write flag -> read only)\n");}
	if ((error_number & ( 1 << 0 )) >> 0) {/*printf("    (Present flag -> page present)\n");*/}else{cons_printf("    (Present flag -> page not present)\n");}
	if (!((error_number & ( 1 << 0 )) >> 0)) { //handle page not present

		//[31-22: main][21-12: sub][11-0: offset] 
		main_table_idx = error_addr >> 22; //index is first 10 bits
		sub_table_idx = (error_addr << 10) >> 22;

		//should search
			for (i=0; i<DRAM_PAGE_COUNT; i++) {
				if (DRAM[i].owner == -1) {
					DRAM[i].owner = error_pid;
					MyBzero((char*)DRAM[i].addr, 0x1000);
					//main_table = (int*)pcb[error_pid].trans_table;
					//main_table += main_table_idx;
if (main_table_idx==512) {
					sub_table = (int*)(0xe01000 + (sub_table_idx*4));
} else {
					sub_table = (int*)(0xe02000 + (sub_table_idx*4));
}
					//sub_table = (int*)(main_table + (sub_table_idx*4));
					*sub_table = DRAM[i].addr;
					*sub_table |= 0x3;			
					break;
				}
			}
			if (i >= DRAM_PAGE_COUNT) {
				//if it can't fix it, kill the process	
				ExitISR();
				cons_printf("PID: %d was killed\n", error_pid);
			}
	}
} //end of function

