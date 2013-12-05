/**
 * Copyright (c) 2013 by Tapti Palit, Kaustubh Gharpure. All rights reserved.
 */

#ifndef _STDIO_H
#define _STDIO_H

#include<common.h>

#define ZERO 0
#define std_in 0
#define std_out 1
#define std_err 2

int scanf(const char *format, ...);
int printf(const char *format, ...);

void putchar(const char);
void putint(s32int, int);
void puts(const char*);

#endif
