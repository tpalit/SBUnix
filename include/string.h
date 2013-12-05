
#ifndef _STRING_H
#define _STRING_H

#include<stdlib.h>

#define MAX_TOKENS 5 // for the strtok function

void trimpath(char*);
void form_full_path(char*, char*, char*);
int strcmp(char*, char*);
void strcpy(char*, char*);
void* memcpy(void*, void*, int);
void trimspaces(char *);
void tokanize(char *,char *,char *);
int strlen(char*);
int isnullstring(char*);
char** strtok(char*);
#endif
