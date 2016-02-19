// proc.c, 159
// processes are coded here

#include "spede.h"      // for IO_DELAY() needed here below
#include "extern.h"     // for currently-running pid needed below
#include "proc.h"       // for prototypes of process functions

void LoadRun() {         // this is not real
   if running_pid is 0:
      call IdleProc()   // as if loads it and it runs
   else (all other processes)
      call UserProc()   // as if loads it and it runs
}

void IdleProc() {
   show msg on PC: "IdleProc (PID 0) runs.\n"
   loop 1666667 times calling IO_DELAY()
}

void UserProc() {
   show msg on target PC:
      cons_printf("UserProc (PID %d) runs.\n", running_pid);
   loop 1666667 times calling IO_DELAY()
}
