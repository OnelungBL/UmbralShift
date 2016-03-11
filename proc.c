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
   //product_sem_id = SemGet(INITIAL_SEMAPHORE_LIMIT); //get "product_sem_id" by calling SemGet()
   //product_count = 0; //clear "product_count"
   for(;;){ //loop forever
	Sleep(1); //sleep for a second
	if(cons_kbhit()){ //poll key
	   key = cons_getchar(); 
	   switch(key){
		case 'p': //if 'p' is pressed, call StartProc() to create a producer process
                  StartProc(ProducerProc);
		  break;
		case 'c': //if 'c' is pressed, call StartProc() to create a consumer process
		   StartProc(ConsumerProc);
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

void ProducerProc() {
   static int count;
   int i;
   int my_pid;
   msg_t my_msg;
   my_pid = GetPid();
   for(;;){ //loop forever:
     my_msg.recipient = 0;
     my_msg.data = my_pid * 100 + count++;
     cons_printf("\n++ Producer (%d) producing data %d...\n", my_pid, my_msg.data);
     for(i=0; i<3333333; i++) { //busy loop for 2 seconds: for(i=0; i<3333333; i++) IO_DELAY();
       IO_DELAY();
     }
     MsgSnd(&my_msg);
   }
}

void ConsumerProc() {
  int i;
  int my_pid;
  msg_t my_msg;
  my_pid = GetPid();
  for(;;) { //loop forever
    my_msg.recipient = 0;
    MsgRcv(&my_msg);
     for(i=0; i<3333333; i++) { //busy loop for 2 seconds: for(i=0; i<3333333; i++) IO_DELAY();
       IO_DELAY();
     }    
  } 
}

