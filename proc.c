// proc.c, 159
// processes are coded here

#include "spede.h"      // for IO_DELAY() needed here below
#include "extern.h"     // for currently-running pid needed below
#include "proc.h"       // for prototypes of process functions
#include "syscall.h"

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
   for(;;){ //loop forever
	Sleep(1); //sleep for a second
	if(cons_kbhit()){ //poll key
	   key = cons_getchar(); 
	   switch(key){
		case 'p': //if 'p' pressed, send a greeting message to PrintDriver (PID 2)
                  PrinterDriver();
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

NEED TO EDIT
void PrintDriver() {  //FROM INSTRUCTOR WEBSITE
   int i, code;
   char *p;

//request for a semaphore printing_semaphore, limit 0.

// reset printer (check printer power, cable, and paper), it will jitter
   outportb(LPT1_BASE+LPT_CONTROL, PC_SLCTIN);   // CONTROL reg, SeLeCT INterrupt
   code = inportb(LPT1_BASE+LPT_STATUS);         // read STATUS
   loop 50 times of IO_DELAY();                  // needs delay
   outportb(LPT1_BASE+LPT_CONTROL, PC_INIT|PC_SLCTIN|PC_IRQEN); // IRQ ENable
   Sleep for a second                            // needs time resetting

   forever loop:
      receive a message                          // get if msg to print
      cons_printf a notification msg (match how demo runs)

      set p to point to start of character string in message
      while "what p points to" is not null/empty/(char)0, then:
         outportb(LPT1_BASE+LPT_DATA, *p);       // write char to DATA reg
         code = inportb(LPT1_BASE+LPT_CONTROL);  // read CONTROL reg
         outportb(LPT1_BASE+LPT_CONTROL, code|PC_STROBE); // write CONTROL, STROBE added
         do 50 times of IO_DELAY();              // needs delay
         outportb(LPT1_BASE+LPT_CONTROL, code);  // send back original CONTROL

         semaphore-wait on the printing semaphore

         move p to next character to print
      end while "what p...
   end forever loop
end PrintDriver()

