/*
 * Copyright (c) 2013 by Tapti Palit. All rights reserved.
 *
 */

#include<sys/timer.h>
#include<common.h>
#include<stdio.h>
/* The timer count since boot */
u64int ticks = 0; 

void find_time_since_boot(time_struct*);
void init_timer(int hz);

void init_timer(int hz)
{
	int divisor = 1193180/hz;
	u8int lo_byte = (u8int)divisor & 0xFF;
	u8int hi_byte = (u8int)(divisor >> 8) & 0xFF;
	__asm__ __volatile__ ("mov $0x36, %%ax;"
			      "outb %%al, $0x43;"
			      "mov %0, %%al;"
			      "outb %%al, $0x40;"
			      "mov %0, %%al;"
			      "outb %%al, $0x40;"
			      ::"r"(lo_byte),"r"(hi_byte));
}

void find_time_since_boot(time_struct* time_s)
{
	/* 
	   Expected frequency is 100 Hz,
	   So we'll be getting one tick every
	   10 ms.
	*/
	time_s->hours = ticks / 360000;
	time_s->minutes = (ticks % 360000)/6000;
	time_s->seconds = (ticks % 6000)/100;
}
