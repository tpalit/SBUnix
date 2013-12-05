/*
 * The process manager. 
 */
/**
 * Copyright (c) 2013 by Tapti Palit, Kaustubh Gharpure. All rights reserved.
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

/**
 * A process can wait on an event - such as child exiting, 
 * or disk read or user entering command.
 */
typedef u8int event_type;
#define CHLD_EXT_EVT 0
#define CMD_ENT_EVT 1

struct event_struct
{
	u64int pid; /* The id of the process waiting on this event. */
	event_type event;
	void* event_opt_info; /* Optional information - e.g. child process */
};

typedef struct event_struct event_struct;

/*
 * The data structure to store all information about
 * a task. 
 */
struct task_struct
{
	char identifier;
	u64int pid;
	struct task_struct* parent_task;
	s32int time_slices;
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
	u8int num_children;
	u8int waiting; /* Whether this task is waiting */
	u8int waiton_spec_child_exit; /* pid of the specific child whose completion this task is waiting for */
	u32int wait_time_slices; /* Remaining time this task should wait. */
};

typedef struct task_struct task_struct;


void add_to_ready_list(task_struct* );

/**
 * Macro version of add_to_ready_list
 */
#define madd_to_ready_list(task_struct_ptr)\
{\
	task_struct* ready_list_ptr = READY_LIST;\
	while(ready_list_ptr != NULL) {\
		if (task_struct_ptr == ready_list_ptr) {\
			return;\
		}\
		ready_list_ptr = ready_list_ptr->next;\
	}\
	ready_list_ptr = READY_LIST;\
	task_struct_ptr->next = NULL;\
	if(ready_list_ptr == NULL){\
		READY_LIST = task_struct_ptr;\
	} else {\
		while(ready_list_ptr->next != NULL) {\
			ready_list_ptr = ready_list_ptr->next;\
		}\
		ready_list_ptr->next = task_struct_ptr;\
	}\
}

void remove_from_ready_list(task_struct* );

/** 
 * Macro version.
 */
/**
 * Removes the task from the ready list.
 */
#define mremove_from_ready_list(task_struct_ptr)				\
	{								\
	task_struct* curr_task = task_struct_ptr;			\
	task_struct* ready_list_ptr = READY_LIST;			\
	task_struct* prev_ptr = NULL;					\
	/* Find the task in the list and remove it. */			\
	if (ready_list_ptr != NULL){					\
		if(curr_task == ready_list_ptr) {			\
			if (prev_ptr != NULL){				\
				prev_ptr->next = curr_task->next;	\
			}						\
			curr_task->next = NULL;				\
		}							\
		prev_ptr = ready_list_ptr;				\
		ready_list_ptr = ready_list_ptr->next;			\
	} else {							\
		panic("READY_LIST is empty! This should never, ever happen!"); \
	}								\
}

void add_to_zombie_list(task_struct* );
void add_to_sleeping_list(task_struct*, event_struct*);
void remove_from_sleeping_list(task_struct*);
/*
void add_event(event_struct*);
void remove_event(event_struct*);
*/

void schedule(void);
void schedule_on_timer(void);
void exit(void);
void sleep(u32int);
void wait(void);
void waitpid(u64int);
void wait_on_read(void);
void sendpid(char *);
void create_kernel_process(task_struct*, u64int);
void create_user_process(task_struct*, u64int);

void reinit_user_process(task_struct*, u64int);

#endif
