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

/* Flag to communicate if the current task did a exit() call */
u8int current_inactive = 0;

/* Global variable for allotting process ids. */
u32int PROC_ID_TOP = 0;

extern pml4_e pml_entries[512];
extern u64int ticks;
extern cr3_reg cr3_register;
extern tss_struct tss_entry; /* The tss_entry.rsp0 has to be updated after each task switch */

/* Whether the scheduler has been inited */
u8int scheduler_inited = 0;


/**
 * Adds a task_struct to the ready list. 
 */
void add_to_ready_list(task_struct* task_struct_ptr)
{
	task_struct* ready_list_ptr = READY_LIST;
	/*
	 * First check if the process is already there. 
	 * @HACK - ideally, the code shouldn't try to add a process twice.
	 * but it is happening!
	 */
	while(ready_list_ptr != NULL) {
		if (task_struct_ptr == ready_list_ptr) {
			return;
		}
		ready_list_ptr = ready_list_ptr->next;
	}
	ready_list_ptr = READY_LIST;
	/* The new process should be added at the end, so it's next should be NULL. */
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

/**
 * Removes the task from the ready list.
 */
void remove_from_ready_list(task_struct* task_struct_ptr)
{
	task_struct* curr_task = task_struct_ptr;
	task_struct* ready_list_ptr = READY_LIST;
	task_struct* prev_ptr = NULL;
	/* Find the task in the list and remove it. */
	if (ready_list_ptr != NULL){
		if(curr_task == ready_list_ptr) {
			if (prev_ptr != NULL){
				prev_ptr->next = curr_task->next;
			}
			curr_task->next = NULL;
		}
		prev_ptr = ready_list_ptr;
		ready_list_ptr = ready_list_ptr->next;
	} else {
		panic("READY_LIST is empty! This should never, ever happen!");
	}
}

/**
 * The process added to the zombie list. 
 */
void add_to_zombie_list(task_struct* task_struct_ptr)
{
	task_struct* zombie_list_ptr = ZOMBIE_LIST;
	/* The new process should be added at the end, so it's next should be NULL. */
	task_struct_ptr->next = NULL;
	if(zombie_list_ptr == NULL){
		ZOMBIE_LIST = task_struct_ptr;
	} else {
		while(zombie_list_ptr->next != NULL) {
			zombie_list_ptr = zombie_list_ptr->next;
		}
		zombie_list_ptr->next = task_struct_ptr;
	}	
}

/**
 * Add to the sleeping list. 
 */
void add_to_sleeping_list(task_struct* task_struct_ptr, event_struct* event_ptr)
{
	task_struct* sleeping_list_ptr = SLEEPING_LIST;
	/* The new process should be added at the end, so it's next should be NULL. */
	task_struct_ptr->next = NULL;
	if(sleeping_list_ptr == NULL){
		SLEEPING_LIST = task_struct_ptr;
	} else {
		while(sleeping_list_ptr->next != NULL) {
			sleeping_list_ptr = sleeping_list_ptr->next;
		}
		sleeping_list_ptr->next = task_struct_ptr;
	}	
	if (event_ptr != NULL){
		task_struct_ptr->waiting_on = event_ptr;
	}
}

/**
 * Remove a task from the sleeping list. 
 */
void remove_from_sleeping_list(task_struct* task_struct_ptr)
{
	task_struct* sleeping_list_ptr = SLEEPING_LIST;
	task_struct* prev_ptr = NULL;
	// Find the task in the list and remove it.
	if (sleeping_list_ptr != NULL){
		if(task_struct_ptr  == sleeping_list_ptr) {
			if (prev_ptr != NULL){
				prev_ptr->next = task_struct_ptr->next;
			}
			task_struct_ptr->next = NULL;
		}
		prev_ptr = sleeping_list_ptr;
		sleeping_list_ptr = sleeping_list_ptr->next;
	} 
}

/**
 * On every tick, go through the sleeping list, to see if any 
 * sleeping process should be woken up. 
 * This should be invoked from schedule_on_timer() only and 
 * not from schedule().
 */
/*
#define mprocess_sleeping_list_on_tick()				\
	{								\
		task_struct* sleeping_list_ptr = SLEEPING_LIST;		\
		task_struct* prev_ptr = NULL;				\
		while(sleeping_list_ptr != NULL){			\
			if (sleeping_list_ptr->wait_time_slices > 0){	\
				sleeping_list_ptr->wait_time_slices--;	\
				if (sleeping_list_ptr->wait_time_slices <=0){ \
					sleeping_list_ptr->wait_time_slices = 0; \
					if (prev_ptr != NULL){		\
						prev_ptr->next = sleeping_list_ptr->next; \
					} else {			\
						SLEEPING_LIST = NULL;	\
					}				\
					madd_to_ready_list(sleeping_list_ptr); \
				}					\
			}						\
			prev_ptr = sleeping_list_ptr;			\
			sleeping_list_ptr = sleeping_list_ptr->next;	\
		}							\
	}
*/

#define mprocess_sleeping_list_on_tick()				\
	{								\
		task_struct* sleeping_list_ptr = SLEEPING_LIST;		\
		task_struct* ready_task = NULL;				\
		task_struct* prev_ptr = NULL;				\
		while(sleeping_list_ptr != NULL){			\
			if(sleeping_list_ptr->wait_time_slices > 0) {	\
				sleeping_list_ptr->wait_time_slices--;	\
			}						\
			if (sleeping_list_ptr->wait_time_slices <=0	\
			    && sleeping_list_ptr->waiton_chld_exit == 0){ \
				/* To remove */				\
				ready_task = sleeping_list_ptr;		\
				ready_task->wait_time_slices = 0;	\
				if (prev_ptr != NULL){			\
					prev_ptr->next = ready_task->next; \
				} else {				\
					/* Start of the list */		\
					SLEEPING_LIST = ready_task->next; \
				}					\
				sleeping_list_ptr = sleeping_list_ptr->next; \
				madd_to_ready_list(ready_task);		\
				/* prev_ptr doesn't change */		\
			} else {					\
				prev_ptr = sleeping_list_ptr;		\
				sleeping_list_ptr = sleeping_list_ptr->next; \
			}						\
		}							\
	}								\
	
/**
 * Cooperative multitasking's schedule. 
 * Identical to the timer code, except that it doesn't acknowledge interrupts and
 * that it handles the schedule() after the exit() and has to deal with weird cases.
 * If we had used the generic interrupt framework, we wouldn't have had to do this.
 */
void schedule()
{
	u64int old_rsp;
	__asm__ __volatile__(
			     "pushq %rax\n\t"
			     "pushq %rbx\n\t"
			     "pushq %rcx\n\t"
			     "pushq %rdx\n\t"
			     "pushq %rbp\n\t"
			     "pushq %rsi\n\t"
			     "pushq %rdi\n\t"
     			     "pushq %r8\n\t"
			     "pushq %r9\n\t"
			     "pushq %r10\n\t"
			     "pushq %r11\n\t"
			     "pushq %r12\n\t"
			     "pushq %r13\n\t"
			     "pushq %r14\n\t"
			     "pushq %r15\n\t"
			     "pushq %rdi\n\t"
			     "pushq %rsi\n\t"
			     "pushq %rdx\n\t");

	__asm__ __volatile__("movq %%rsp, %[old_rsp]": [old_rsp] "=r"(old_rsp));
	ticks++; /* Global variable */
	if(READY_LIST != NULL) {
		if (!scheduler_inited){
			/* 
			 * The first time schedule is called, we 
			 * just load the kernel stack and do a iretq
			 * since the switching mechanism wasn't in effect earlier.
			 * And the other pushes aren't on the register yet. 
			 */
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
			__asm__ __volatile__(
					     "popq %rdx\n\t"
					     "popq %rsi\n\t"
					     "popq %rdi\n\t"
					     "popq %r15\n\t"
					     "popq %r14\n\t"
					     "popq %r13\n\t"
					     "popq %r12\n\t"
					     "popq %r11\n\t"
					     "popq %r10\n\t"
					     "popq %r9\n\t"
					     "popq %r8\n\t"
					     "popq %rdi\n\t"
					     "popq %rsi\n\t"
					     "popq %rbp\n\t"
					     "popq %rdx\n\t"
					     "popq %rcx\n\t"
					     "popq %rbx\n\t"
					     "popq %rax\n\t");
			__asm__ __volatile__("iretq\n\t");
		} else {
			prev = CURRENT_TASK;
			next = READY_LIST;
			if(!current_inactive) {
				/* If we're putting the process to sleep, we're saving
				   the stack pointer at the entry of do_sleep() */
				__asm__ __volatile__(
						     "movq %[old_rsp], %[prev_rsp]\n\t"
						     :[prev_rsp] "=m" (prev->rsp_register)
						     :[old_rsp] "r" (old_rsp));
			}
			__asm__ __volatile__("movq %[next_rsp], %%rsp\n\t"
					     ::[next_rsp] "m" (next->rsp_register));
			CURRENT_TASK = READY_LIST;
			/* Add prev to the end of the READY_LIST, unless this is marked as inactive */
			if (!current_inactive) {
				add_to_ready_list(prev);
				prev = NULL;
				READY_LIST = READY_LIST->next;
			} else {
				READY_LIST = READY_LIST->next;
				/* If removing prev makes this the last entry, then add it back */
				if (READY_LIST == NULL){
					add_to_ready_list(next);
				}
				current_inactive = 0;
			}
			__asm__ __volatile__(
					     "movq %0, %%cr3\n\t"
					     ::"r"(next->cr3_register));			
			tss_entry.rsp0 = (u64int)&next->kernel_stack[KERNEL_STACK_SIZE-1];
			__asm__ __volatile__(
					     "popq %rdx\n\t"
					     "popq %rsi\n\t"
					     "popq %rdi\n\t"
					     "popq %r15\n\t"
					     "popq %r14\n\t"
					     "popq %r13\n\t"
					     "popq %r12\n\t"
					     "popq %r11\n\t"
					     "popq %r10\n\t"
					     "popq %r9\n\t"
					     "popq %r8\n\t"
					     "popq %rdi\n\t"
					     "popq %rsi\n\t"
					     "popq %rbp\n\t"
					     "popq %rdx\n\t"
					     "popq %rcx\n\t"
					     "popq %rbx\n\t"
					     "popq %rax\n\t");
			__asm__ __volatile__("iretq\n\t");
		}
	} 
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
 * @TODO - The handling for timer interrupt doesn't go through the typical
 * common handler routines. Doing that shouldn't be very tough. 
 */
void schedule_on_timer(void)
{
	/**
	 * Calling a function pushes %rbx on to the stack which 
	 * corrupts everything. So, no function call, till old_rsp is read.
	 * We'll read once at the start of the function and once after pushing 
	 * the registers
	 */
	mprocess_sleeping_list_on_tick();
	u64int old_rsp;
	if (!scheduler_inited || (READY_LIST != NULL && CURRENT_TASK->time_slices <= 0)) {

		__asm__ __volatile__(
				     "pushq %rax\n\t"
				     "pushq %rbx\n\t"
				     "pushq %rcx\n\t"
				     "pushq %rdx\n\t"
				     "pushq %rbp\n\t"
				     "pushq %rsi\n\t"
				     "pushq %rdi\n\t"
				     "pushq %r8\n\t"
				     "pushq %r9\n\t"
				     "pushq %r10\n\t"
				     "pushq %r11\n\t"
				     "pushq %r12\n\t"
				     "pushq %r13\n\t"
				     "pushq %r14\n\t"
				     "pushq %r15\n\t"
				     "pushq %rdi\n\t"
				     "pushq %rsi\n\t"
				     "pushq %rdx\n\t");
		__asm__ __volatile__("movq %%rsp, %[old_rsp]": [old_rsp] "=r"(old_rsp));

		ticks++; /* Global variable */
		if (!scheduler_inited){
			/* 
			 * The first time schedule is called, we 
			 * just load the kernel stack and do a iretq
			 * since the switching mechanism wasn't in effect earlier.
			 * And the other pushes aren't on the register yet. 
			 */
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
					     "out %al, $0xA0\n\t");
			__asm__ __volatile__(
					     "popq %rdx\n\t"
					     "popq %rsi\n\t"
					     "popq %rdi\n\t"
					     "popq %r15\n\t"
					     "popq %r14\n\t"
					     "popq %r13\n\t"
					     "popq %r12\n\t"
					     "popq %r11\n\t"
					     "popq %r10\n\t"
					     "popq %r9\n\t"
					     "popq %r8\n\t"
					     "popq %rdi\n\t"
					     "popq %rsi\n\t"
					     "popq %rbp\n\t"
					     "popq %rdx\n\t"
					     "popq %rcx\n\t"
					     "popq %rbx\n\t"
					     "popq %rax\n\t");
			__asm__ __volatile__("iretq\n\t");
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
			add_to_ready_list(prev);
			if(READY_LIST->next != NULL) {
				READY_LIST = READY_LIST->next;
			
			}
			__asm__ __volatile__(
					     "movq %0, %%cr3\n\t"
					     ::"r"(next->cr3_register));			
			tss_entry.rsp0 = (u64int)&next->kernel_stack[KERNEL_STACK_SIZE-1];
			__asm__ __volatile__("mov $0x20, %al\n\t"
					     "out %al, $0x20\n\t"
					     "out %al, $0xA0\n\t");
			__asm__ __volatile__(
					     "popq %rdx\n\t"
					     "popq %rsi\n\t"
					     "popq %rdi\n\t"
					     "popq %r15\n\t"
					     "popq %r14\n\t"
					     "popq %r13\n\t"
					     "popq %r12\n\t"
					     "popq %r11\n\t"
					     "popq %r10\n\t"
					     "popq %r9\n\t"
					     "popq %r8\n\t"
					     "popq %rdi\n\t"
					     "popq %rsi\n\t"
					     "popq %rbp\n\t"
					     "popq %rdx\n\t"
					     "popq %rcx\n\t"
					     "popq %rbx\n\t"
					     "popq %rax\n\t");
			__asm__ __volatile__("iretq\n\t");
		}
	} else {
		/* 
		 * No need to reschedule, continue running the existing process 
		 * Also, no need to decrement the time slice if it is the only task in 
		 * the system. 
		 */
		if (READY_LIST != NULL) {
			CURRENT_TASK->time_slices--;
		}
		__asm__ __volatile__("pushq %rax\n\t");
		__asm__ __volatile__("mov $0x20, %al\n\t"
				     "out %al, $0x20\n\t"
				     "out %al, $0xA0\n\t"
				     "popq %rax\n\t"
				     "iretq\n\t");		
	}
}

/**
 * Mark the current task as inactive and put it in the ZOMBIE list.
 * It will then invoke schedule() to schedule another process.
 */
void exit(void)
{
	current_inactive = 1; /* Don't add CURRENT_TASK back to READY_LIST in schedule() */
	if(CURRENT_TASK->parent_task != NULL){
		CURRENT_TASK->parent_task->waiton_chld_exit = 0; /* @TODO - handle for more children */
	}
	add_to_zombie_list(CURRENT_TASK);
	schedule();
}

/**
 * Mark the current task as inactive and put it in the SLEEPING list.
 * It will then invoke schedule() to schedule another process.
 */
void sleep(u32int sleep_slices)
{
	current_inactive = 1;
	CURRENT_TASK->wait_time_slices = sleep_slices;
	add_to_sleeping_list(CURRENT_TASK, NULL);
	schedule();
}

void wait(void)
{
	current_inactive = 1;
	add_to_sleeping_list(CURRENT_TASK, NULL);
	/* @TODO - Handle for all children */
	CURRENT_TASK->waiton_chld_exit = 1;
	schedule();
}
/**
 * Sets up the kernel process. This includes, setting up the kernel to 
 * something that the timer_interrupt expects - rip, cs, rflags, rsp and ss. 
 */
void create_kernel_process(task_struct* task_struct_ptr, u64int function_ptr)
{
	/* Set up task parameters as per what IRETQ expects*/
	task_struct_ptr->kernel_stack[127] = 0x10;
	task_struct_ptr->kernel_stack[126] = (u64int)&task_struct_ptr->kernel_stack[127];
	task_struct_ptr->kernel_stack[125] = 0x20202;
	task_struct_ptr->kernel_stack[124] = 0x08;
	task_struct_ptr->kernel_stack[123] = function_ptr;

	/* Pretend that the GP registers and one function call is also on the stack */
	int indx = 122;
	for (; indx>105; indx--) {
		task_struct_ptr->kernel_stack[indx] = 0x0;
	}
	task_struct_ptr->rsp_register = (u64int)&task_struct_ptr->kernel_stack[105];

	task_struct_ptr->rip_register = function_ptr;
	task_struct_ptr->time_slices = DEFAULT_TIME_SLICE;
	task_struct_ptr->rflags = DEFAULT_FLAGS;
	task_struct_ptr->next = NULL;
	task_struct_ptr->last_run = NULL;
	task_struct_ptr->pid = PROC_ID_TOP++;
	task_struct_ptr->vm_head = NULL;
	task_struct_ptr->waiting_on = NULL;
	task_struct_ptr->waiton_chld_exit = 0;
	task_struct_ptr->wait_time_slices = 0;
	task_struct_ptr->parent_task = NULL;
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
	task_struct_ptr->kernel_stack[126] = BAD_ADDRESS; /* Overwrite later */
	task_struct_ptr->kernel_stack[125] = DEFAULT_FLAGS;
	task_struct_ptr->kernel_stack[124] = 0x1b;
	task_struct_ptr->kernel_stack[123] = function_ptr;

	/* Pretend that the GP registers and one function call is also on the stack */
	int indx = 122;
	for (; indx>105; indx--) {
		task_struct_ptr->kernel_stack[indx] = 0x0;
	}
	task_struct_ptr->time_slices = DEFAULT_TIME_SLICE;
	task_struct_ptr->rsp_register = (u64int)&task_struct_ptr->kernel_stack[105];
	task_struct_ptr->rip_register = function_ptr;
	task_struct_ptr->rflags = 0x20202;
	task_struct_ptr->next = NULL;
	task_struct_ptr->last_run = NULL;
	task_struct_ptr->pid = PROC_ID_TOP++;
	task_struct_ptr->parent_task = NULL;
	task_struct_ptr->vm_head = NULL;
	task_struct_ptr->waiting_on = NULL;
	task_struct_ptr->wait_time_slices = 0;
	task_struct_ptr->waiton_chld_exit = 0;
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
			create_pml4_e(&pml_entries_ptr[i], (u64int)page_addr->phys_addr, 0x0, 0x03, 0x00);
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

/**
 * Reinitialize basic task attributes on overlaying.
 */
void reinit_user_process(task_struct* task_struct_ptr, u64int function_ptr) 
{
	task_struct_ptr->kernel_stack[122] = function_ptr;
	task_struct_ptr->time_slices = DEFAULT_TIME_SLICE;
	task_struct_ptr->rip_register = function_ptr;
	task_struct_ptr->rflags = DEFAULT_FLAGS;
	task_struct_ptr->waiting_on = NULL;
	task_struct_ptr->waiton_chld_exit = 0;
	task_struct_ptr->wait_time_slices = 0;
	/* Don't copy the CODE or DATA vmas as they'll be reloaded. */
	vm_struct* vm_ptr = task_struct_ptr->vm_head;
	vm_struct* vm_new_ptr = NULL;
	while(vm_ptr != NULL){
		if (vm_ptr->vm_type != CODE_VMA &&
		    vm_ptr->vm_type != DATA_VMA){
			vm_struct* new_vm = (vm_struct*)kmalloc(sizeof(vm_struct));
			new_vm->vm_type = vm_ptr->vm_type;
			new_vm->vm_start = vm_ptr->vm_start;
			new_vm->vm_end = vm_ptr->vm_end;
			new_vm->vm_next = vm_ptr->vm_next;
			if (vm_new_ptr != NULL){
				vm_new_ptr->vm_next = new_vm;
			} else {
				vm_new_ptr = new_vm;
			}
		}
		vm_ptr = vm_ptr->vm_next;
	}
	
}
