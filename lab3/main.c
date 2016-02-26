//*********************************************
//Name: Darren Takemoto
//Phase 0, Lab 3 - Timer Lab
//main.c
//*********************************************
#include <spede/stdio.h>
#include <spede/flames.h>
#include <spede/machine/io.h>
#include <spede/machine/proc_reg.h>
#include <spede/machine/seg.h>
#include <spede/machine/pic.h>

#include "entry.h" // needs addr of TimerEntry

typedef void (* func_ptr_t)();
struct i386_gate *IDT_ptr;

void SetEntry(int entry_num, func_ptr_t func_ptr) {
	struct i386_gate *gateptr = &IDT_ptr[entry_num];
	fill_gate(gateptr, (int)func_ptr, get_cs(), ACC_INTR_GATE, 0);
}

int main() {
	int i;
	
	IDT_ptr = get_idt_base();	// ge where IDT is
	cons_printf("IDT is at %u.\n", IDT_ptr);
	
	SetEntry(32, TimerEntry);	// prime IDT entry
	outportb(0x21, ~1);		// 0x21 is PIC mask, ~1 is mask
	asm("sti");
	
	while(1) {			// IO_DELAY() delays CPU by .65 micro sec:
		for(i=0; i<1666000; i++) IO_DELAY(); // asm("inb $0x80,%%al" : : : "eax");
		cons_putchar('X');	// show 'X'
		if(cons_kbhit()) break;	// break loop if KB is pressed
	}
	
	return 0;			// main() ends
}
