// proc.c, 159
// processes are coded here

#include "spede.h"      // for IO_DELAY() needed here below
#include "extern.h"     // for currently-running pid needed below
#include "proc.h"       // for prototypes of process functions
#include "syscall.h"
#include "toolfunc.h"
#include "isr.h"

void IdleProc() {
	int i;
	while (1) {
		cons_printf("0 ");
		for(i=0; i<1666667; i++) {
			IO_DELAY();
		}
	}
}

void InitProc() {
	char key;
	msg_t print_msg;
        SemPostISR(printing_semaphore);
	MyBzero((char *)&print_msg, sizeof(msg_t));
	MyStrcpy("Greetings from Team UmbralShift!\n", print_msg.data);
	while (1) { //loop forever
		Sleep(1); //sleep for a second
		if(cons_kbhit()){ //poll key
			key = cons_getchar(); 
			switch(key){
			case 'p': //if 'p' pressed, send a greeting message to PrintDriver (PID 2)
                                print_msg.recipient = PROC_PRINT;
  				MsgSnd(&print_msg);
                               outportb(0x20,0x67);
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
	msg_t print_msg;
	printing_semaphore = SemGet(INITIAL_PRINT_SEMAPHORE_LIMIT);
	// reset printer (check printer power, cable, and paper), it will jitter
	outportb(LPT1_BASE+LPT_CONTROL, PC_SLCTIN);   // CONTROL reg, SeLeCT INterrupt
	code = inportb(LPT1_BASE+LPT_STATUS);         // read STATUS
	for (i=0; i<50; i++) { IO_DELAY(); } // needs delay
	outportb(LPT1_BASE+LPT_CONTROL, PC_INIT|PC_SLCTIN|PC_IRQEN); // IRQ ENable
	for(i=0; i<1666667; i++) { IO_DELAY(); }// needs time resetting
	while (1) { //forever loop
		MsgRcv(&print_msg); // get if msg to print                     
		cons_printf("PrintDriver (PID 2) now prints...\n");
		p = print_msg.data;
		while (*p != '\0') {
			outportb(LPT1_BASE+LPT_DATA, *p);       // write char to DATA reg
			code = inportb(LPT1_BASE+LPT_CONTROL);  // read CONTROL reg
			outportb(LPT1_BASE+LPT_CONTROL, code|PC_STROBE); // write CONTROL, STROBE added
			for (i=0; i<50; i++) { IO_DELAY(); } // needs delay
			outportb(LPT1_BASE+LPT_CONTROL, code);  // send back original CONTROL
                        outportb(0x20,0x67);
			SemWait(printing_semaphore);
			p++;
		} //end while p != '\0'
                
		
	} //end forever loop
}

void ShellProc() { //FROM INSTRUCTOR WEBSITE
        msg_t my_msg;
        char a_string[101];                  // a handy string

        int BAUD_RATE, divisor, my_pid;
        my_pid = GetPid();  // get my PID
        //   initialize the interface port-data structure (port_data_t port_data):
        MyBzero((char *)&port_data, sizeof(port_data_t));
        port_data.TX_semaphore = SemGet(Q_LEN);
        port_data.RX_semaphore = SemGet(0);
        port_data.echo_mode=1;
        port_data.TXRDY=1;
        port_data.stdin_pid = my_pid + 1;
        port_data.stdout_pid = my_pid + 2;

        // A. set baud rate 9600
        BAUD_RATE = 9600;              // Mr. Baud invented this
        divisor = 115200 / BAUD_RATE;  // time period of each baud
        outportb(COM2_IOBASE+CFCR, CFCR_DLAB);          // CFCR_DLAB 0x80
        outportb(COM2_IOBASE+BAUDLO, LOBYTE(divisor));
        outportb(COM2_IOBASE+BAUDHI, HIBYTE(divisor));
        // B. set CFCR: 7-E-1 (7 data bits, even parity, 1 stop bit)
        outportb(COM2_IOBASE+CFCR, CFCR_PEVEN|CFCR_PENAB|CFCR_7BITS);
        outportb(COM2_IOBASE+IER, 0);
        // C. raise DTR, RTS of the serial port to start read/write
        outportb(COM2_IOBASE+MCR, MCR_DTR|MCR_RTS|MCR_IENABLE);
        IO_DELAY();
        outportb(COM2_IOBASE+IER, IER_ERXRDY|IER_ETXRDY); // enable TX, RX events
        IO_DELAY();

		//MsgRecipient only needs to be set for MsgSnd.
		MyStrcpy("\n\n\nHello World!  Team UmbralShift here!\n\n", my_msg.data);        
        my_msg.recipient = port_data.stdout_pid;
        MsgSnd(&my_msg); //(completion timing reply, content don't care)
        MsgRcv(&my_msg);

        while(1) {
                MyStrcpy("Team UmbralShift, enter something: ", my_msg.data);
                my_msg.recipient=port_data.stdout_pid;
                MsgSnd(&my_msg);
                MsgRcv(&my_msg);  //(completion timing reply, content don't care)
                my_msg.recipient=port_data.stdin_pid;
                MsgSnd(&my_msg);

                MsgRcv(&my_msg); //(completion timing reply, content don't care)
                MyStrcpy(my_msg.data, a_string);
                
                MyStrcpy("Just Entered -> ", my_msg.data);
                my_msg.recipient=port_data.stdout_pid;
                MsgSnd(&my_msg);
                MsgRcv(&my_msg);

                MyStrcpy(a_string, my_msg.data);
                my_msg.recipient=port_data.stdout_pid;
                MsgSnd(&my_msg);
                MsgRcv(&my_msg);

                MyStrcpy("\n", my_msg.data);
                my_msg.recipient=port_data.stdout_pid;
                MsgSnd(&my_msg);
                MsgRcv(&my_msg);

        }
}

void StdinProc() { //FROM INSTRUCTOR WEBSITE
        msg_t my_msg;
        char *p;

        while (1) {
                MsgRcv(&my_msg);
                p = my_msg.data;
                while (1) {
                char ch;
                        SemWait(port_data.RX_semaphore);
                        //ch = (char) &port_data.RX_buffer;
                        ch = DeQ(&port_data.RX_buffer);
                        if (ch == '\r') break;  // delimiter encountered
                        *p++ = ch;
                }
                *p = '\0';   // add NUL to terminate msg.data
                my_msg.recipient=my_msg.sender;
                MsgSnd(&my_msg);
        }
}

void StdoutProc() { //FROM INSTRUCTOR WEBSITE
        msg_t my_msg;
        char *p;
        while (1) {
                //receive my_msg (from user shell)
                MsgRcv(&my_msg);
                //char ptr p points to my_msg.data
                p = my_msg.data;

                //loop until p points to null:
                while (*p) {
                        //semaphore wait on TX_semaphore of port data
                        SemWait(port_data.TX_semaphore);
                        //enqueue what p points to to TX_buffer of port data
                        EnQ(*p, &port_data.TX_buffer);
                        if (*p == '\n') {
                                //semaphore wait on TX_semaphore of port data
                                SemWait(port_data.TX_semaphore);
                                //enqueue '\r' to TX_buffer of port data
                                EnQ('\r', &port_data.TX_buffer);
                        }
                        //issue syscall "TipIRQ3();" to manually start IRQ-3 event
                       TipIRQ3();
                        //advance p
                        p++;
                }
                //set my_msg.recipient to my_msg.sender
                my_msg.recipient = my_msg.sender;
                //send my_msg back
                MsgSnd(&my_msg);
        }
}

