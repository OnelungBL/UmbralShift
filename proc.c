// proc.c, 159
// processes are coded here

#include "spede.h"      // for IO_DELAY() needed here below
#include "extern.h"     // for currently-running pid needed below
#include "proc.h"       // for prototypes of process functions

// void LoadRun(int running_pid) {         // this is not real
//    if (running_pid == 0) {
//       IdleProc();
//    } else {
//       UserProc(running_pid);
//    }
// }

void IdleProc() {
   int i;
   for(;;) {
      cons_printf("IdleProc (PID 0) runs.\n");
      for(i=0; i<1666667; i++) {
         IO_DELAY();
      }
   }
}

void UserProc(int running_pid) {
   int i;
   for(;;) {
      cons_printf("UserProc (PID %d) runs.\n", running_pid);
      for(i=0; i<1666667; i++) {
         IO_DELAY();
      }
   }
}
