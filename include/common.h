/*
 * This file contains common typedefs to make life
 * a little easier.
 * 
 * Copyright (c) 2013 by Tapti Palit. All rights reserved.
 */

#ifndef COMMON_H
#define COMMON_H

typedef unsigned long int u64int;
typedef long int s64int;
typedef unsigned int u32int;
typedef int s32int;
typedef unsigned short u16int;
typedef short s16int;
typedef unsigned char u8int;
typedef char s8int;

void outb(u16int, u8int);
void panic(char*);
void dump_regs(void);

#define KERN_VIR_START 0xffffffff80000000UL

#define NULL 0
#endif

