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

void UserProc() {
  for(;;) {
    cons_printf("%d ", GetPid());
    Sleep(GetPid() % 3 + 1);
  }
}
