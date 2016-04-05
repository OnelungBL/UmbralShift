// toolfunc.c, 159

#include "spede.h"
#include "typedef.h"
#include "extern.h"

void MyBzero(char *p, int byte_size) {
	int i = 0;
	for(i=0; i<byte_size; i++) {
		p[i]='\0';
	}
}

void EnQ(int pid, q_t *p) {
	if (p->len < Q_LEN) {
		//p->q[p->len] = pid;
		p->q[p->tail] = pid;
		p->tail++;
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
	p->tail--;
	for (i=0; i<p->len; i++) {
		p->q[i]=p->q[i+1];
	}
	for(i=p->len; i<Q_LEN; i++){
		p->q[i] = -1;
	}
	return val;
}

void MsgEnQ(msg_t *msg_ptr, msg_q_t *msg_q_ptr) {
	if (msg_q_ptr->len >= Q_LEN) {
		cons_printf("Panic: msg_q is full, cannot enqueue!\n");
		return;
	}
	msg_q_ptr->msg[msg_q_ptr->tail] = *msg_ptr;
	msg_q_ptr->tail++;
	msg_q_ptr->len++;
	if (msg_q_ptr->tail >= Q_LEN) {
		msg_q_ptr->tail = 0;
	}
}

msg_t * MsgDeQ(msg_q_t *msg_q_ptr) {
	msg_t *ret_ptr;
	if (msg_q_ptr->len <= 0) {
		cons_printf("Panic: msg_q is empty, cannot dequeue!\n");
		return '\0';
	} else {
		ret_ptr = &msg_q_ptr->msg[msg_q_ptr->head];
		msg_q_ptr->len--;
		msg_q_ptr->head++;
		if (msg_q_ptr->head == (Q_LEN-1)) {
			msg_q_ptr->head = 0;
		}
		return ret_ptr;
	}
	return '\0';

}

void MyStrcpy(char *src, char *dest) {
	while (*src != '\0') {
			*dest++ = *src++;
	}
	*dest = '\0';
}
