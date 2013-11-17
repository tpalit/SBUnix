/*
 * Copyright (c) 2013 by Tapti Palit. All rights reserved.
 */

#ifndef _STDIO_H
#define _STDIO_H

#include <common.h>

int printf(const char *format, ...);
/* TODO - All this should go into the include/sys */
int kprintf(const char *format, ...);
int scanf(const char *format, ...);

void putchar(const char);
void putint(s32int, int);
void puts(const char*);
void do_bkspace(void);
#endif
