/*
 * Copyright (c) 2013 by Tapti Palit. All rights reserved.
 */

#ifndef _STDIO_H
#define _STDIO_H

#include <common.h>
#define ZERO 0
#define std_in 0
#define std_out 1
#define std_err 2
char STDIN[100];
volatile int writebuff;
int scanf(const char *format, ...);
int printf(const char *format, ...);
/* @TODO - All this should go into the include/sys */
int kprintf(const char *format, ...);
int scanf(const char *format, ...);

void putchar(const char);
void putint(s32int, int);
void puts(const char*);
void do_bkspace(void);
#endif
