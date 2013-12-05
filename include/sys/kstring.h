#ifndef _STRING_H
#define _STRING_H

int kstrcmp(char*, char*);
int kstrcmpsz(char*, char*);
void kstrcpy(char*, char*);
void trimpath(char*);
void addpath(char*, char*);
void trim(char*, char*,char*);
void kstrcpysz(char*, char*,int);
void* kmemcpy(void*, void*, int);
int kstrlen(char*);
void num2str(int,char b[]);
void strconcat(char *,char*);
int strcmp(char*, char*);
void strcpy(char*, char*);
void* memcpy(void*, void*, int);
#endif
