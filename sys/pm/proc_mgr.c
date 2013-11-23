
/* 
 * The process manager. 
 */
#include<sys/kmalloc.h>
#include<sys/proc_mgr.h>
#include<sys/vm_mgr.h>
#include<sys/desc_tbls.h>
#include<common.h>

/* The process lists. The task at the head of the READY_LIST should always be executed next.*/
task_struct* READY_LIST = NULL;
task_struct* SLEEPING_LIST = NULL;
task_struct* ZOMBIE_LIST = NULL;

task_struct* CURRENT_TASK = NULL;
task_struct* prev = NULL;
task_struct* next = NULL;

u32int PROC_ID_TOP = 0;

extern pml4_e pml_entries[512];
extern u64int ticks;
extern cr3_reg cr3_register;
extern tss_struct tss_entry; /* The tss_entry.rsp0 has to be updated after each task switch */

/* Whether the scheduler has been inited */
u8int scheduler_inited = 0;
void __switch_to()
{
	/*
	task_struct* temp = NULL;
	temp = prev;
	prev = next;
	next = temp;
	*/
}

/**
 * Adds a task_struct to the ready list. 
 */
void add_to_ready_list(task_struct* task_struct_ptr)
{
	task_struct* ready_list_ptr = READY_LIST;
	// The new process should be added at the end, so it's next should be NULL.
	task_struct_ptr->next = NULL;
	if(ready_list_ptr == NULL){
		READY_LIST = task_struct_ptr;
	} else {
		while(ready_list_ptr->next != NULL) {
			ready_list_ptr = ready_list_ptr->next;
		}
		ready_list_ptr->next = task_struct_ptr;
	}
}

void schedule()
{
	/*
	if (!scheduler_inited){
	
		if (READY_LIST == NULL) {
			return;
		}
	        prev = READY_LIST;
		READY_LIST = READY_LIST->next;
		CURRENT_TASK = prev;
		init_schedule();
	} else {
		prev = CURRENT_TASK;
		CURRENT_TASK = READY_LIST;
	        next = READY_LIST;
	
		add_to_ready_list(prev);
		READY_LIST = READY_LIST->next;
		switch_to(prev,next);
	}*/
}

/*
 * This is the heart of the scheduling code. 
 * Note: To avoid unnecessary return addresses on the stack, 
 * this method gets directly called on a timer interrupt happening. 
 * The context switching code relies on the kernel stack per process.
 * While creating a new process this is initialized to the order in
 * which IRETQ pops - rip, cs, rflags, rsp and ss. 
 * 
 * The first time this or schedule() is invoked, extra things
 * need to be done. So that's handled separately.
 *
 * In short, it does the following things -
 * 1. Save the old kernel stack
 * 2. Load the new kernel stack
 * 3. Moves the top of the kernel stack to the tss.rsp0
 *
 * @TODO - Code is in extended assembly, which is probably a bad idea.
 * Converting to pure assembly is a better idea as suggested by Yongming. 
 */
void schedule_on_timer(void)
{
	u64int old_rsp;
	__asm__ __volatile__("movq %%rsp, %[old_rsp]": [old_rsp] "=r"(old_rsp));
	ticks++; /* Global variable */
	if(READY_LIST != NULL) {
		if (!scheduler_inited){
			if (READY_LIST == NULL) {
				return;
			}
			prev = READY_LIST;
			READY_LIST = READY_LIST->next;
			CURRENT_TASK = prev;
			scheduler_inited = 1;
			__asm__ __volatile__(
					     "movq %[prev_rsp], %%rsp\n\t"
					     :
					     :[prev_rsp] "m" (prev->rsp_register));
			__asm__ __volatile__(
					     "movq %0, %%cr3\n\t"
					     ::"r"(prev->cr3_register));
			tss_entry.rsp0 = (u64int)&prev->kernel_stack[KERNEL_STACK_SIZE-1];
			__asm__ __volatile__("mov $0x20, %al\n\t"
					     "out %al, $0x20\n\t"
					     "out %al, $0xA0\n\t"
					     "iretq\n\t");
		} else {
			prev = CURRENT_TASK;
			next = READY_LIST;
			__asm__ __volatile__(
					     "movq %[old_rsp], %[prev_rsp]\n\t"
					     "movq %[next_rsp], %%rsp\n\t"
					     :[prev_rsp] "=m" (prev->rsp_register)
					     :[old_rsp] "r" (old_rsp),
					     [next_rsp] "m" (next->rsp_register));
			CURRENT_TASK = READY_LIST;
			/* Add prev to the end of the READY_LIST */
			add_to_ready_list(prev);
			READY_LIST = READY_LIST->next;
			__asm__ __volatile__(
					     "movq %0, %%cr3\n\t"
					     ::"r"(next->cr3_register));			
			tss_entry.rsp0 = (u64int)&next->kernel_stack[KERNEL_STACK_SIZE-1];
			__asm__ __volatile__("mov $0x20, %al\n\t"
					     "out %al, $0x20\n\t"
					     "out %al, $0xA0\n\t"
					     "iretq\n\t");
		}
	} else {
		/* 
		 * Code should never reach here, as READY_LIST should never be null. 
		 * We'll always have an idle kernel process.
		 */
		__asm__ __volatile("mov $0x20, %al\n\t"
				   "out %al, $0x20\n\t"
				   "out %al, $0xA0\n\t"
				   "iretq\n\t");
	}

}

/**
 * Sets up the kernel process. This includes, setting up the kernel to 
 * something that the timer_interrupt expects.
 */
void create_kernel_process(task_struct* task_struct_ptr, u64int function_ptr)
{
	/* Set up task parameters as per what IRETQ expects*/
	task_struct_ptr->kernel_stack[127] = 0x10;
	task_struct_ptr->kernel_stack[126] = (u64int)&task_struct_ptr->kernel_stack[127];
	task_struct_ptr->kernel_stack[125] = 0x20202;
	task_struct_ptr->kernel_stack[124] = 0x08;
	task_struct_ptr->kernel_stack[123] = function_ptr;

	task_struct_ptr->rsp_register = (u64int)&task_struct_ptr->kernel_stack[123];
	task_struct_ptr->rip_register = function_ptr;
	task_struct_ptr->rflags = 0x20202;
	task_struct_ptr->next = NULL;
	task_struct_ptr->last_run = NULL;
	task_struct_ptr->proc_id = PROC_ID_TOP++;
	task_struct_ptr->vm_head = NULL;
	/* Set up the process address space */
	/* This has to be aligned on 0x1000 boundaries and need the physical address */
	phys_vir_addr* page_addr = get_free_phys_page();
	pml4_e* pml_entries_ptr = (pml4_e*)page_addr->vir_addr;

	/* Map the higher memory by copying the PML4E*/
	int i = 0;

	/* Create blank PML4E */
	for(i=0; i<512; i++){
		if (i==PML4_REC_SLOT) {
			/* The recursive mapping */
			create_pml4_e(&pml_entries_ptr[i], (u64int)page_addr->phys_addr, 0x0, 0x07, 0x00);
		} else {
			pml_entries_ptr[i] = pml_entries[i];			

		}
	}
	cr3_reg process_cr3;
	create_cr3_reg(&process_cr3, (u64int)page_addr->phys_addr, 0x00, 0x00);
	task_struct_ptr->cr3_register = process_cr3;
	/* Add to the ready list */
	add_to_ready_list(task_struct_ptr);
}

void create_user_process(task_struct* task_struct_ptr, u64int function_ptr) 
{
	/* Set up task parameters as per what IRETQ expects*/
	task_struct_ptr->kernel_stack[127] = 0x23;
	task_struct_ptr->kernel_stack[126] = (u64int)&task_struct_ptr->kernel_stack[127];
	task_struct_ptr->kernel_stack[125] = 0x20202;
	task_struct_ptr->kernel_stack[124] = 0x1b;
	task_struct_ptr->kernel_stack[123] = function_ptr;

	task_struct_ptr->rsp_register = (u64int)&task_struct_ptr->kernel_stack[123];
	task_struct_ptr->rip_register = function_ptr;
	task_struct_ptr->rflags = 0x20202;
	task_struct_ptr->next = NULL;
	task_struct_ptr->last_run = NULL;
	task_struct_ptr->proc_id = PROC_ID_TOP++;
	task_struct_ptr->vm_head = NULL;
	/* Set up the process address space */
	/* This has to be aligned on 0x1000 boundaries and need the physical address */
	phys_vir_addr* page_addr = get_free_phys_page();
	pml4_e* pml_entries_ptr = (pml4_e*)page_addr->vir_addr;

	/* Map the higher memory by copying the PML4E*/
	int i = 0;

	/* Create blank PML4E */
	for(i=0; i<512; i++){
		if (i==PML4_REC_SLOT) {
			/* The recursive mapping */
			create_pml4_e(&pml_entries_ptr[i], (u64int)page_addr->phys_addr, 0x0, 0x07, 0x00);
		} else {
			pml_entries_ptr[i] = pml_entries[i];			

		}
	}

	cr3_reg process_cr3;
	create_cr3_reg(&process_cr3, (u64int)page_addr->phys_addr, 0x00, 0x00);
	task_struct_ptr->cr3_register = process_cr3;
	/* Add to the ready list */
	add_to_ready_list(task_struct_ptr);
}
