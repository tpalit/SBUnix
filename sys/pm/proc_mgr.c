/* 
 * The process manager. 
 */
#include<sys/kmalloc.h>
#include<sys/proc_mgr.h>
#include<stdio.h>
#include<common.h>

/* The process lists. The task at the head of the READY_LIST should always be executed next.*/
task_struct* READY_LIST = NULL;
task_struct* SLEEPING_LIST = NULL;
task_struct* ZOMBIE_LIST = NULL;

task_struct* CURRENT_TASK = NULL;
task_struct* prev = NULL;
task_struct* next = NULL;

extern u64int ticks;
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

void timer_interrupt2(void)
{
	__asm__ __volatile__("mov $0x20, %al\n\t"
			   "out %al, $0x20\n\t"
			   "out %al, $0xA0\n\t"
			   "iretq\n\t");
}

/*
 * The handler for the timer interrupt. 
 * This will print the time since boot on the
 * bottom right corner of the screen.
 */
void timer_interrupt(void)
{
	u64int old_rsp;
	__asm__ __volatile__("movq %%rsp, %[old_rsp]": [old_rsp] "=r"(old_rsp));
	ticks++; /* Global variable */

	if(READY_LIST != NULL) {
		if (!scheduler_inited){
			/* The first time schedule gets invoked - additional processing needs done. */
			if (READY_LIST == NULL) {
				return;
			}
			prev = READY_LIST;
			READY_LIST = READY_LIST->next;
			CURRENT_TASK = prev;
			scheduler_inited = 1;
			/* Need to switch the kernel stack to that of the first process. */
			/* IRETQ pops in the order of rip, cs, rflags, rsp and ss. */
			__asm__ __volatile__(
					     "movq %[prev_rsp], %%rsp\n\t"
					     :
					     :[prev_rsp] "m" (prev->rsp_register));

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
			__asm__ __volatile__("mov $0x20, %al\n\t"
					     "out %al, $0x20\n\t"
					     "out %al, $0xA0\n\t"
					     "iretq\n\t");

		}
	} else {
		/* Code should never reach here, as READY_LIST should never be null */
		__asm__ __volatile("mov $0x20, %al\n\t"
				   "out %al, $0x20\n\t"
				   "out %al, $0xA0\n\t"
				   "iretq\n\t");
	}

}

void init_schedule()
{
	scheduler_inited = 1;
	/* Need to switch the kernel stack to that of the first process. */
	__asm__ __volatile("movq %[prev_sp], %%rsp\n\t"::[prev_sp] "m" (prev->rsp_register));
	__asm__ __volatile("movq %[prev_bp], %%rbp\n\t"::[prev_bp] "m" (prev->rsp_register));
}

void create_new_process(task_struct* task_struct_ptr, u64int function_ptr)
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
	/* Add to the ready list */
	add_to_ready_list(task_struct_ptr);
}
