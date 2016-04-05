// proc.c, 159
// processes are coded here

#include "spede.h"      // for IO_DELAY() needed here below
#include "extern.h"     // for currently-running pid needed below
#include "proc.h"       // for prototypes of process functions
#include "syscall.h"
#include "toolfunc.h"

void IdleProc() {
	int i;
	for(;;) {
		cons_printf("0 ");
		for(i=0; i<1666667; i++) {
			IO_DELAY();
		}
	}
}

void InitProc() {
	char key;
	msg_t msg;
	MyStrcpy("Greetings from Team UmbralShift!\n", msg.data);
	for(;;){ //loop forever
		Sleep(1); //sleep for a second
		if(cons_kbhit()){ //poll key
			key = cons_getchar(); 
			switch(key){
			case 'p': //if 'p' pressed, send a greeting message to PrintDriver (PID 2)
				MsgSnd(2, &msg);
				break;
			case 'b': //if 'b: is pressed, execute GDB breakpoint
				breakpoint();
				break;
			case 'x': // if 'x' is pressed, just exit
				exit(0);
			}	
		}
	}
}

void PrintDriver() {  //FROM INSTRUCTOR WEBSITE
	int i, code;
	char *p;
	msg_t msg;
	printing_semaphore = SemGet(INITIAL_PRINT_SEMAPHORE_LIMIT);
	// reset printer (check printer power, cable, and paper), it will jitter
	outportb(LPT1_BASE+LPT_CONTROL, PC_SLCTIN);   // CONTROL reg, SeLeCT INterrupt
	code = inportb(LPT1_BASE+LPT_STATUS);         // read STATUS
	for (i=0; i<50; i++) { IO_DELAY(); } // needs delay
	outportb(LPT1_BASE+LPT_CONTROL, PC_INIT|PC_SLCTIN|PC_IRQEN); // IRQ ENable
	for(i=0; i<1666667; i++) { IO_DELAY(); }// needs time resetting

	for(;;) { //forever loop
		MsgRcv(&msg); // get if msg to print                     
		cons_printf("PrintDriver (PID 2) now prints...");
		p = msg.data;
		while (p != '\0') {
			outportb(LPT1_BASE+LPT_DATA, *p);       // write char to DATA reg
			code = inportb(LPT1_BASE+LPT_CONTROL);  // read CONTROL reg
			outportb(LPT1_BASE+LPT_CONTROL, code|PC_STROBE); // write CONTROL, STROBE added
			for (i=0; i<50; i++) { IO_DELAY(); } // needs delay
			outportb(LPT1_BASE+LPT_CONTROL, code);  // send back original CONTROL
			SemWait(printing_semaphore);
			p = p + 1;
		} //end while p != '/0'
		
	} //end forever loop
}

