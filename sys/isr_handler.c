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
#include<sys/vm_mgr.h>
#include<sys/kstring.h>
#include<sys/terminal.h>

extern u64int ticks;

extern void find_time_since_boot(time_struct*);
extern void itoa(u64int, char*, int);
extern task_struct* READY_LIST;

extern task_struct* CURRENT_TASK;
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
	volatile u64int faulting_address = 0x0;
	volatile u64int error_code = 0x0;
	vm_struct* vma_to_map = NULL;
	u8int okay_to_map = 0;
	__asm__ __volatile__("movq %%cr2, %[cr2_register]\n\t":[cr2_register]"=r"(faulting_address));
	__asm__ __volatile__("movq %%r10, %[error_code]\n\t":[error_code]"=r"(error_code));
	/* If the kernel faults or there's a fault in accessing a Present page, stop. */
	kprintf("The error code = %x\n", error_code);
	/*
	u64int* pml4_ptr = (u64int*)PML4_ENTRY(faulting_address);
	pml4_e* pml4_entry_ptr = pml4_ptr+PML4E_OFFSET((u64int)faulting_address);
	u64int* pdp_ptr = (u64int*)PDP_ENTRY(faulting_address);
	pdp_e* pdp_entry_ptr = pdp_ptr+PDPT_OFFSET((u64int)faulting_address);
	u64int* pd_ptr = (u64int*)PD_ENTRY(faulting_address);
	pd_e* pd_entry_ptr = pd_ptr+PD_OFFSET((u64int)faulting_address);
	u64int* pt_ptr = (u64int*)PT_ENTRY(faulting_address);
	pt_e* pt_entry_ptr = pt_ptr+PT_OFFSET((u64int)faulting_address);
	kprintf("pml4_entry_ptr = %p\n", *pml4_entry_ptr);
	kprintf("pdp_entry_ptr = %p\n", *pdp_entry_ptr);
	kprintf("pd_entry_ptr = %p\n", *pd_entry_ptr);
	kprintf("pt_entry_ptr = %p\n", *pt_entry_ptr);
	*/
	dump_regs();
	panic("Page fault!");
	if (faulting_address >= KERN_VIR_START || (error_code & 0x01)) { 
		/* Page fault in Kernel */
		dump_regs();
		panic("Bad type of Page fault Happened!");
	} else {
		/* @TODO - Assume that the CURRENT_TASK has caused the page fault. 
		*  Check if this is always true.
		*/
		vm_struct* vm_struct_ptr = CURRENT_TASK->vm_head;
		while (vm_struct_ptr != NULL){
			if (faulting_address >= vm_struct_ptr->vm_start && 
			    faulting_address <= vm_struct_ptr->vm_end){
				vma_to_map = vm_struct_ptr;
				okay_to_map = 1;
				break;
			}
			vm_struct_ptr = vm_struct_ptr->vm_next;
		}
	}
	if(okay_to_map){
		kmmap((void*)vma_to_map->vm_start, 
		      vma_to_map->vm_end-vma_to_map->vm_start,
		      0, 0, 0, 0);
	}
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
