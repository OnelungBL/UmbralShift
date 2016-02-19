// proc.c, 159
// processes are coded here

#include "spede.h"      // for IO_DELAY() needed here below
#include "extern.h"     // for currently-running pid needed below
#include "proc.h"       // for prototypes of process functions

void LoadRun() {         // this is not real
   if (running_pid == 0) {
      IdleProc();
   } else {
      UserProc();
   }
}

void IdleProc() {
   int i;
   
   cons_printf("IdleProc (PID 0) runs.\n");
   for(i=0; i<1666667; i++) {
      IO_DELAY();
   }
}

void UserProc() {
   int i;
   
   cons_printf("UserProc (PID %d) runs.\n", running_pid);
   for(i=0; i<1666667; i++) {
      IO_DELAY();
   }
}
