// toolfunc.c, 159

#include "spede.h"
#include "typedef.h"
#include "extern.h"

void MyBzero(char *p, int byte_size) {
  //Zero out the contents of the pointer/string ?
  int i = 0;
  for(i=0; i<byte_size; i++) {
    p[i]='\0';
  }
}

void EnQ(int pid, q_t *p) {
//  int i;
//does not wrap; can only hold 20 processes during runtime.
  if (p.len < 20) {
    p.q[p.len] = pid;
    p.len++;
    p.tail = p.len;
  } else {
    cons_printf("queue is already full");
    return;
  }
//  for (i = 0; i<Q_LEN; i++) {
//    if (p.q[i] == '\0') {
//      p.q[i] = pid;
//      p.tail = i;
//      p.len = i;
//    }
//  }
// ?????????????????????????????????????????????????
// show error msg and return if queue's already full
// needs coding
// ?????????????????????????????????????????????????
}

int DeQ(q_t *p) { // return -1 if q is empty
// ?????????????????????????????????????????????????
// needs coding
// ?????????????????????????????????????????????????
}

