/*
 * This file will contain the actual interrupt handlers.
 * Check isr_common.s for details on how the code reaches here.
 *
 * Copyright (c) 2013 by Tapti Palit. All rights reserved.
 * 
 */
#include<sys/timer.h>
#include<common.h>
#include<stdio.h>
#include<sys/proc_mgr.h>
#include<sys/terminal.h>

extern u64int ticks;

extern void find_time_since_boot(time_struct*);
extern void itoa(u64int, char*, int);
extern task_struct* READY_LIST;

/*
 * Flags to indicate if shift and control keys
 * are pressed.
 */
/*
static u8int is_shift_pressed = 0;
static u8int is_ctrl_pressed = 0;
*/
/*
 * This is the scan scode map that will translate
 * between the scan codes sent by the keyboard
 * to the ASCII character.
 */

/*
static char scan_code_map[128] = 
	{
		' ', ' ', '1', '2', '3', '4', '5', '6', '7', 
		'8', '9', '0', '-', '=', '\b','\t', 'q', 'w', 
		'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', 
		']', ' ', ' ', 'a', 's', 'd', 'f', 'g', 'h', 
		'j', 'k', 'l', ';', '\'', '`', ' ', '\\', 'z',
		'x', 'c', 'v', 'b', 'n', 'm', '.', '/', ' '
	};
*/

void print_time_on_screen(time_struct*);

/*
 * The handler for divide by zero. 
 */
void isr_handler_body_0(void)
{
	dump_regs();
	panic("Divide By Zero Happened!");
}

/* Invalid TSS exception */
void isr_handler_body_10(void)
{
	dump_regs();
	panic("Invalid TSS Exception!");
}

/* 
 * The GP fault handler. 
 */
void isr_handler_body_13(void)
{
	dump_regs();
	panic("General Protection Fault Happened!");

}

/*
 * The PF handler.
 */
void isr_handler_body_14(void)
{
       	dump_regs();
	panic("Page fault Happened!");

}


/*
 * The handler for keyboard interrupt.
 */
void isr_handler_body_33(void)
{
	/*
	 * The keyboard writes the scan code
	 * at 0x60. So read it.
	 */

	u8int scan_code;
	__asm__("inb $0x60, %%al\n\t"
		"mov %%al, %0\n\t"
		:"=r"(scan_code));
	terminal(scan_code);
}

void print_time_on_screen(time_struct* time_s)
{
	/* 
	 * To print on the bottom right corner,
	 * we need to first figure out what is the
	 * bottom right corner.
	 * For now, hard coding it!
	 */
	/* 
	 * @FIX_THIS - Terrible code. Find a better way.
	 */
       	volatile char *bt_rt_crner = (volatile char*)0xB8F90;
	char hours[2];
	char mins[2];
	char secs[2];
	itoa(time_s->hours, hours, 10);
	itoa(time_s->minutes, mins, 10);
	itoa(time_s->seconds, secs, 10);
	*bt_rt_crner++ = hours[0];
	*bt_rt_crner++ = 0x07;
	*bt_rt_crner++ = hours[1];
	*bt_rt_crner++ = 0x07;
	*bt_rt_crner++ = ':';
	*bt_rt_crner++ = 0x07;
	*bt_rt_crner++ = mins[0];
	*bt_rt_crner++ = 0x07;
	*bt_rt_crner++ = mins[1];
	*bt_rt_crner++ = 0x07;
	*bt_rt_crner++ = ':';
	*bt_rt_crner++ = 0x07;
	*bt_rt_crner++ = secs[0];
	*bt_rt_crner++ = 0x07;
	*bt_rt_crner++ = secs[1];
	*bt_rt_crner++ = 0x07;
	

}
