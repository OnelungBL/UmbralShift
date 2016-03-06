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
  if (p->len < Q_LEN) {
     p->q[p->len] = pid;
    //p->q[p->tail] = pid;
    //p->tail++;
    p->len++;
  } else {
    cons_printf("queue is already full");
    return;
  }
}

int DeQ(q_t *p) { // return -1 if q is empty
  int i;
  int val;
  if (p->len == 0) {
    return -1;
  }
  val = p->q[0];
  p->len--;
  for (i=0; i<p->len; i++) {
    p->q[i]=p->q[i+1];
  }
  for(i=p->len; i<Q_LEN; i++){
    p->q[i] = -1;
  }
  return val;
}
