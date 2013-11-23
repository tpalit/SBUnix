/*
 * The process manager. 
 */

#ifndef PROCMGR_H
#define PROCMGR_H


#include<common.h>
#include<sys/vm_mgr.h>
#define KERNEL_STACK_SIZE 128
#define DEFAULT_TIME_SLICE 5
#define DEFAULT_FLAGS 0x20202

typedef struct regs_struct regs_struct;
typedef struct regs_struct* regs_struct_ptr;

/*
 * The data structure to store all information about
 * a task. 
 */
struct task_struct
{
	char identifier;
	u64int proc_id;
	u32int time_slices;
	/* The registers for this process */
	u64int pml4_entry_base;
	u64int kernel_stack[KERNEL_STACK_SIZE];
	u64int rip_register;
	u64int rsp_register;
	u64int rflags;
	struct task_struct* next; /* The next process in the process list - either the ACTIVE/SLEEPING/ZOMBIE */
	struct task_struct* last_run; /* The process that ran last */
	vm_struct* vm_head;
	cr3_reg cr3_register;
};

typedef struct task_struct task_struct;

void add_to_ready_list(task_struct* );
void remove_from_ready_list(task_struct* );
void add_to_zombie_list(task_struct* );

void schedule(void);
void schedule_on_timer(void);
void exit(void);

void create_kernel_process(task_struct*, u64int);
void create_user_process(task_struct*, u64int);

/*
 * The previous and next task structs.
 * The previous will be pushed on to the stack so that it's reference can be found
 * when restoring. 
 */
#define switch_to(prev, next) \
	__asm__ __volatile__(\
			     "pushfq\n\t"\
			     "pushq %%rbp\n\t"\
	                     "pushq %[my_last]\n\t"\
			     "pushq %%rax\n\t"\
			     "pushq %%rbx\n\t"\
			     "pushq %%rcx\n\t"\
			     "pushq %%rdx\n\t"\
			     "movq %%rsp, %[prev_sp]\n\t" /* Store the current rsp */\
			     "movq %[next_sp], %%rsp\n\t" /* Restore a previously switched out sp */\
			     "movq $1f, %[prev_ip]\n\t" /* Save IP to the 1 label */\
			     "pushq %[next_ip]\n\t" /* Push the previously switched out IP */\
			     "jmp __switch_to\n\t" /* Call switch_to function so that it pops the next_ip pushed */\
			     "1:\t"\
			     "popq %%rdx\n\t"\
			     "popq %%rcx\n\t"\
			     "popq %%rbx\n\t"\
			     "popq %%rax\n\t"\
                             "popq %[new_last]\n\t"\
			     "popq %%rbp\n\t"\
			     "popfq\n\t"\
			     :[prev_sp] "=m" (prev->rsp_register),\
		              [prev_ip] "=m" (prev->rip_register),\
		              [new_last] "=m" (prev)\
                             :[my_last] "m" (prev),\
                              [next_sp] "m" (next->rsp_register),\
			      [next_ip] "m" (next->rip_register));\

#endif
