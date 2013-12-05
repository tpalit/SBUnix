/**
 * Copyright (c) 2013 by Tapti Palit, Kaustubh Gharpure. All rights reserved.
 */


#ifndef _KSTDIO_H
#define _KSTDIO_H

#include<common.h>

char STDIN[100];
volatile int writebuff;

int kprintf(const char *format, ...);
int scanf(const char *format, ...);
void ltoa(u64int, char*);
void putchar(const char);
void putint(s32int, int);
void puts(const char*);
void do_bkspace(void);

#endif
