// toolfunc.h, 159

#ifndef _TOOLFUNC_H
#define _TOOLFUNC_H

#include "typedef.h" // q_t needs be defined in code below

void MyBzero(char *, int);
int DeQ(q_t *);
void EnQ(int, q_t *);
void MsgEnQ(msg_t *, msg_q_t *);
msg_t * MsgDeQ(msg_q_t *);
void MyStrcpy(char *, char *);
void MyMemcpy(char *, char *, int);
int MyStrcmp(char *, char *, int);
int MyStrlen(char *);
#endif

