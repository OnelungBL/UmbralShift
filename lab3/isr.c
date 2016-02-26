//***********************************
//NAME: Darren Takemoto
//Phase 0, Lab 3 - Timer Lab
//isr.c
//***********************************
#include <spede/flames.h>
#include <spede/machine/io.h>
#include <spede/machine/pic.h>

// 2-byte (unsigned short) ptr points to video memory location
// row 10, col 40, from upper-left corner(b8000)
unsigned short *vid_mem_ptr = (unsigned short *) 0xB8000+10*80+39;

#define SPACE (0x0f00 + ' ')		// 0x0f00, VGA attr: bold W on B

void TimerISR() {			// evoked from TimerEntry
	static unsigned short ch = SPACE;  // start with space char
	static int timer_count = 0;	// count timer-intr occurences
	
// dismiss timer intr (IRQ 0), otherwise, new intr signal from timer
// won't be recognized by CPU since circuit uses edge-trigger flipflop
	outportb(0x20, 0x60);  // 0x20 is PIC control reg, 0x60 dismisses IRQ 0
	
	if(++timer_count % 75 == 0) {	// every .75 sec
		*vid_mem_ptr = ch;	// display ascii value
		ch++;			// next char in ASCII table
		
		if(ch == SPACE + 95)	// once beyond last printable char ~
			ch = SPACE;	// move it back to 1st printable ' '
	}
}
