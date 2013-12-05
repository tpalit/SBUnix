/*
 * This file will contain the actual interrupt handlers.
 * Check isr_common.s for details on how the code reaches here.
 *
 * Copyright (c) 2013 by Tapti Palit. All rights reserved.
 * 
 */
#include<sys/timer.h>
#include<common.h>
#include<sys/kstdio.h>
#include<sys/proc_mgr.h>
#include<sys/pm_mgr.h>
#include<sys/vm_mgr.h>
#include<sys/kstring.h>
#include<sys/terminal.h>

u8int backup_page[PAGE_SIZE];

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
	u8int okay_to_regrow = 0;
	__asm__ __volatile__("movq %%cr2, %[cr2_register]\n\t":[cr2_register]"=r"(faulting_address));
	__asm__ __volatile__("movq %%r10, %[error_code]\n\t":[error_code]"=r"(error_code));
	
	/* Get the PTE entry for the faulting address */
	u64int* ptr = (u64int*)PT_ENTRY(faulting_address);
	pt_e* pte_entry = ptr + PT_OFFSET(faulting_address); 

	if(is_supervisor(*pte_entry) || faulting_address >= KERN_VIR_START) {
		/* Page fault in kernel. Panic */
		dump_regs();
		panic("Page fault in kernel!");
	} else if (is_present(*pte_entry) && is_readonly(*pte_entry) && is_cow(*pte_entry)) {
		/* 
		 * Copy on write!
		 * Steps - 
		 * 1. Allocate a new physical page.
		 * 2. Read the faulting page and copy its contents to the new page.
		 * 3. Update the PTE entry to point to the new physical page.
		 * 4. Unset cow and readonly.
		 */
		u64int fa_start_page = (u64int)faulting_address & 0xfffffffffffff000;
		phys_vir_addr* page_ptr = get_free_phys_page();
		kmemcpy((void*)page_ptr->vir_addr, (void*)fa_start_page, PAGE_SIZE);
		/* Clearing the PTE */
		create_pt_e(pte_entry, 0x0, 0x0, 0x06, 0x0);
		set_base_addr(pte_entry, page_ptr->phys_addr);
		set_present(pte_entry);
		unset_cow(pte_entry);
		unset_readonly(pte_entry);
	} else if (!is_present(*pte_entry)){
		/* Perhaps page faulted on a mallocked page */
		vm_struct* vm_struct_ptr = CURRENT_TASK->vm_head;
		while (vm_struct_ptr != NULL){
			if (faulting_address >= vm_struct_ptr->vm_start && 
			    faulting_address <= vm_struct_ptr->vm_end){
				vma_to_map = vm_struct_ptr;
				okay_to_map = 1;
				break;
			}
			if (vm_struct_ptr->vm_type == STACK_VMA
			    && faulting_address <= vm_struct_ptr->vm_end+16
			    && vm_struct_ptr->vm_end-vm_struct_ptr->vm_start+16 < STACK_LIMIT){
				/* Auto grow the stack */
				okay_to_regrow = 1;
				break;
			}
			vm_struct_ptr = vm_struct_ptr->vm_next;
		}
		if(okay_to_map){
			kmmap((void*)vma_to_map->vm_start, 
			      vma_to_map->vm_end-vma_to_map->vm_start,
			      0, 0, 0, 0);
		} else if (okay_to_regrow){
			/* Allocate another page to stack */
			vm_struct_ptr = CURRENT_TASK->vm_head;
			while(vm_struct_ptr != NULL) {
				if (vm_struct_ptr->vm_type == STACK_VMA) {
					kmmap((void*)vm_struct_ptr->vm_start,
					      vm_struct_ptr->vm_end+PAGE_SIZE,
					      0,0,0,0);
					vm_struct_ptr->vm_end += PAGE_SIZE;
				}
			}
		} else {
			/*
			dump_regs();
			panic("SEGFAULT - shouldn't kill the kernel, though!");
			*/
			dump_regs();
			kprintf("SEGFAULT - Process killed!\n");
			exit();
		}
	} else {
		dump_regs();
		panic("Unhandled page fault!");
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
