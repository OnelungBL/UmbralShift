// toolfunc.c, 159

#include "spede.h"
#include "typedef.h"
#include "extern.h"

void MyBzero(char *p, int byte_size) {
	int i;
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
		msg_q_ptr->head++;
		msg_q_ptr->len--;
		if (msg_q_ptr->head == (Q_LEN)) {
			msg_q_ptr->head = 0;
		}
		return ret_ptr;
	}
	return '\0';
	
}

void MyStrcpy(char *dest, char *src) {
	while (*src != '\0') {
		*dest++ = *src++;
	}
	*dest = '\0';
}

// copy "size" bytes from char pointer "src" to char pointer "dest"
void MyMemcpy(char *dest, char *src, int size) {
	int i;
	for (i=0; i<size; i++) {
		dest[i] = src[i];
	}
        i++;
	dest[i] = '\0';
}

// compare, byte by byte, two given strings upto "size" bytes;
// return 1 if they're the same, otherwise 0
int MyStrcmp(char *p, char *q, int size) {
	int i;
	int retValue;
	retValue = 1;
	for (i=0; i<size; i++) {
		if (p[i] != q[i]) {
			retValue = 0;
			break;
		}
	}
	return retValue;
}

// returns the length of the string given, assumed null terminated
int MyStrlen(char *s) {
	int i;
        i = 0;
	while (s[i]) {
		i++;
	}
	return i;
}
