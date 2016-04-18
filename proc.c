// proc.c, 159
// processes are coded here

#include "spede.h"      // for IO_DELAY() needed here below
#include "extern.h"     // for currently-running pid needed below
#include "proc.h"       // for prototypes of process functions
#include "syscall.h"
#include "toolfunc.h"
#include "isr.h"
#include "FileService.h"

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
	MyStrcpy(print_msg.data, "Greetings from Team UmbralShift!\n");
	while (1) { //loop forever
		Sleep(1); //sleep for a second
		if(cons_kbhit()){ //poll key
			key = cons_getchar(); 
			switch(key){
				case 'b': //if 'b: is pressed, execute GDB breakpoint
				breakpoint();
				break;
				case 'x': // if 'x' is pressed, just exit
				exit(0);
			}	
		}
	}
}

void ShellProc() { //FROM INSTRUCTOR WEBSITE
	msg_t my_msg;
	char login[101], password[101];
	
	int BAUD_RATE, divisor, my_pid;
	int FileServicePid;
	FileServicePid = 2;
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
	MyStrcpy(my_msg.data, "\n\n\nHello World!  Team UmbralShift here!\n\n");        
	my_msg.recipient = port_data.stdout_pid;
	MsgSnd(&my_msg); //(completion timing reply, content don't care)
	MsgRcv(&my_msg);
	
	while(1) {                        // run the Demo.dli to see how it works
		while(1) {
			MyStrcpy(my_msg.data, "UmbralShift login > ");
            my_msg.recipient=port_data.stdout_pid;
            MsgSnd(&my_msg);
			MsgRcv(&my_msg);
			MyStrcpy(my_msg.data, login);
			
			MyStrcpy(my_msg.data, "UmbralShift password > ");
            my_msg.recipient=port_data.stdout_pid;
            MsgSnd(&my_msg);
			port_data.echo_mode=0;
			MsgRcv(&my_msg);
			port_data.echo_mode=1;
			MyStrcpy(password, my_msg.data);			
			
			//if login and password compare is equal; break loop and continue
			//otherwise, continue to prompt for username/password
			
			if (MyStrcmp(password, login, 101) == 1) break;
			
			MyStrcpy(my_msg.data, "Invalid password!\n");
            my_msg.recipient=port_data.stdout_pid;
            MsgSnd(&my_msg);
			MsgRcv(&my_msg);
			
		}
		
		while(1) {
			MyStrcpy(my_msg.data, "UmbralShift shell > ");
            my_msg.recipient=port_data.stdout_pid;
            MsgSnd(&my_msg);
			MsgRcv(&my_msg);
			
			if (!my_msg.data[0]) continue; //if pointer is null go back to beginning of while loop
			
			
			if ((MyStrcmp(&my_msg.data[0], "end", (MyStrlen(&my_msg.data[0])*sizeof(char)) == 1)) || (MyStrcmp(&my_msg.data[0], "000", (MyStrlen(&my_msg.data[0])*sizeof(char))) == 1)) break;
			
			if ((MyStrcmp(&my_msg.data[0], "dir", (MyStrlen(&my_msg.data[0])*sizeof(char)) == 1)) || (MyStrcmp(&my_msg.data[0], "111", (MyStrlen(&my_msg.data[0])*sizeof(char))) == 1)) {
				DirSub(my_msg.data, FileServicePid);
				continue;
			}
			
			if ((MyStrcmp(&my_msg.data[0], "cat", (MyStrlen(&my_msg.data[0])*sizeof(char)) == 1)) || (MyStrcmp(&my_msg.data[0], "222", (MyStrlen(&my_msg.data[0])*sizeof(char))) == 1)) {
				CatSub(my_msg.data, FileServicePid);
				continue;
			}			
			
			MyStrcpy(my_msg.data, "What's up?\n");
            my_msg.recipient=port_data.stdout_pid;
            MsgSnd(&my_msg);
			MsgRcv(&my_msg);
		}
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

void DirStr(attr_t *p, char *str) { // build str from attributes in given target
	// my_msg.data has 2 parts: attr_t and target, p+1 points to target
	char *target = (char *)(p + 1);
	
	// build str from attr_t p points to
	sprintf(str, " - - - -  size =%6d    %s\n", p->size, target);
	if ( A_ISDIR(p->mode) ) str[1] = 'D';         // mode is directory
	if ( QBIT_ON(p->mode, A_ROTH) ) str[3] = 'R'; // mode is readable
	if ( QBIT_ON(p->mode, A_WOTH) ) str[5] = 'W'; // mode is writable
	if ( QBIT_ON(p->mode, A_XOTH) ) str[7] = 'X'; // mode is executable
} 

// "dir" command, ShellProc talks to FileServicePid and port_data.stdout_pid
// make sure cmd_str ends with \0: "dir\0" or "dir obj...\0"
void DirSub(char *cmd_str, int FileServicePID) {
	char str[101];
	msg_t my_msg;
	attr_t *p;
	
	// if cmd_str is "dir" assume "dir /\0" (on root dir)
	// else, assume user specified an target after first 4 letters "dir "
	if(cmd_str[3] == ' ') {
		cmd_str += 4; // skip 1st 4 letters "dir " and get the rest: obj...
		} else {
		cmd_str[0] = '/';
		cmd_str[1] = '\0'; // null-terminate the target
	}
	
	// apply standard "check target" protocol
	my_msg.code[0] = CHK_OBJ;
	MyStrcpy(my_msg.data, cmd_str);
	
	my_msg.recipient = FileServicePID;
	MsgSnd(&my_msg);            // send my_msg to FileServicePid
	MsgRcv(&my_msg);            // receive reply
	
	if(my_msg.code[0] != GOOD) {           // chk result code
		MyStrcpy(my_msg.data, "DirSub: CHK_OBJ return code is not GOOD!\n\0");
		my_msg.recipient = port_data.stdout_pid;
		MsgSnd(&my_msg);
		MsgRcv(&my_msg);
		
		return;
	}
	
	p = (attr_t *)my_msg.data;     // otherwise, code[0] good, my_msg has "attr_t," 
	
	if(! A_ISDIR(p->mode)) {       // if it's file, "dir" it
		DirStr(p, str);             // str will be built and returned
		MyStrcpy(my_msg.data, str); // go about since p pointed to my_msg.data
		my_msg.recipient = port_data.stdout_pid;
		MsgSnd(&my_msg);
		MsgRcv(&my_msg);
		
		return;
	}
	
	// otherwise, it's a DIRECTORY! -- list each entry in it in loop.
	// 1st request to open it, then issue reads in loop
	
	// apply standard "open target" protocol
	my_msg.code[0] = OPEN_OBJ;
	MyStrcpy(my_msg.data, cmd_str);
	my_msg.recipient = FileServicePID;
	MsgSnd(&my_msg);
	MsgRcv(&my_msg);
	
	while(1) {                     // apply standard "read obj" protocol
		my_msg.code[0] = READ_OBJ;
		my_msg.recipient = FileServicePID;
		MsgSnd(&my_msg);
		MsgRcv(&my_msg);
		
		if(my_msg.code[0] != GOOD) break; // EOF
		
		// do same thing to show it via STANDOUT
		p = (attr_t *)my_msg.data;
		DirStr(p, str);                // str will be built and returned
		MyStrcpy(my_msg.data, str);
		my_msg.recipient = port_data.stdout_pid;
		MsgSnd(&my_msg);  // show str onto terminal
		MsgRcv(&my_msg);
	}
	
	// apply "close obj" protocol with FileServicePid
	// if response is not good, display an error my_msg via port_data.stdout_pid...
	my_msg.code[0] = CLOSE_OBJ;
	my_msg.recipient = FileServicePID;
	MsgSnd(&my_msg);
	MsgRcv(&my_msg);
	
	if(my_msg.code[0] != GOOD) {
		MyStrcpy(my_msg.data, "Dir: CLOSE_OBJ returns NOT GOOD!\n\0");
		my_msg.recipient = port_data.stdout_pid;
		MsgSnd(&my_msg);
		MsgRcv(&my_msg);
	}
}

// "cat" command, ShellProc talks to FileServicePid and port_data.stdout_pid
// make sure cmd_str ends with \0: "cat file\0"
void CatSub(char *cmd_str, int FileServicePID) {
	msg_t my_msg;
	attr_t *p;
	
	cmd_str += 4; // skip 1st 4 letters "cat " and get the rest
	
	// apply standard "check target" protocol
	my_msg.code[0] = CHK_OBJ;
	MyStrcpy(my_msg.data, cmd_str);
	
	my_msg.recipient = FileServicePID;
	MsgSnd(&my_msg);        // send my_msg to FileServicePid
	MsgRcv(&my_msg);        // receive reply
	
	p = (attr_t *)my_msg.data;      // otherwise, code[0] good, chk attr_t
	
	if(my_msg.code[0] != GOOD || A_ISDIR(p->mode) ) { // if directory
		MyStrcpy(my_msg.data, "Usage: cat [path]filename\n\0");
		my_msg.recipient = port_data.stdout_pid;
		MsgSnd(&my_msg);
		MsgRcv(&my_msg);
		
		return;
	}
	
	// 1st request to open it, then issue reads in loop
	
	// apply standard "open obj" protocol
	my_msg.code[0] = OPEN_OBJ;
	MyStrcpy(my_msg.data, cmd_str);
	my_msg.recipient = FileServicePID;
	MsgSnd(&my_msg);
	MsgRcv(&my_msg);
	
	while(1) {
		// apply standard "read target" protocol
		my_msg.code[0] = READ_OBJ;
		MyStrcpy(my_msg.data, cmd_str);
		my_msg.recipient = FileServicePID;
		MsgSnd(&my_msg);
		MsgRcv(&my_msg);
		
		// did it read OK?
		if (my_msg.code[0]==GOOD) break;
		
		// otherwise, show file content via port_data.stdout_pid
		my_msg.recipient = port_data.stdout_pid;
		MsgSnd(&my_msg);
		MsgRcv(&my_msg);
	}
	
	// apply standard "close target" protocol with FileServicePid
	my_msg.code[0] = CLOSE_OBJ;
	MyStrcpy(my_msg.data, cmd_str);
	my_msg.recipient = FileServicePID;
	MsgSnd(&my_msg);
	MsgRcv(&my_msg);
		
	
	// if return code is not good, show error msg onto terminal
		if (my_msg.code[0]==GOOD) {
		//my_msg.recipient =  port_data.stdout_pid;;
		//MsgSnd(&my_msg);
		//MsgRcv(&my_msg);
		sprintf("%s\n", my_msg.data);
	}
}
