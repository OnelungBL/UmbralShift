// proc.h, 159

#ifndef _PROC_H_
#define _PROC_H_

#include "FileService.h"

void IdleProc();
void InitProc();
void FileService();
void ShellProc();
void StdinProc();
void StdoutProc();
void DirStr(attr_t *, char *);
void DirSub(char *, int);
void CatSub(char *, int);

#endif
