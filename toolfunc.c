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
  if (pid < 20) {
    p->q[p->tail] = pid;
    p->tail++;
    p->len = p->tail - p->head;
  } else {
    cons_printf("queue is already full");
    return;
  }
}

int DeQ(q_t *p) { // return -1 if q is empty
  int i;
  int val;
  if (p->head == p->tail) {
    return -1;
  } else {
    val = p->q[p->head];
    p->tail--;
    for (i=0; i<Q_LEN-1; i++) {
      p->q[i]=p->q[i+1];
    }
    p->q[i]=0;
    p->len = p->tail - p->head;
    
  }
  return val;
}

