/**
 * This file contains the functionalities to register and invoke system calls.
 */


#include<common.h>
#include<syscall.h>
#include<sys/proc_mgr.h>
#include<sys/kstring.h>
#include<stdio.h>

#define SYSCALL_NR 10

extern task_struct* CURRENT_TASK;

/* These will get invoked in kernel mode. */
int do_write(char* s)
{
	/*
	while(*s != '\0'){
		putchar(*s++);
	}
	return 42;
	*/
	kprintf(s);
	return 0;
}

int do_read(char* s)
{
	return 0;
}

int do_malloc(u32int mem_size)
{
	u64int ret_ptr;
	vm_struct* vma_ptr = CURRENT_TASK->vm_head;
        while (vma_ptr != NULL){
		if(vma_ptr->vm_type == HEAP_VMA){
			break;
		}
		vma_ptr = vma_ptr->vm_next;
	}
	if(vma_ptr == NULL){
		panic("A process with no heap found!");
	}
	ret_ptr = vma_ptr->vm_end;
	vma_ptr->vm_end+=mem_size;
	return ret_ptr;
}

void do_exit(void)
{
	exit();
}

void do_sleep(u32int sleep_time)
{
	sleep(sleep_time);
}

/* Set up the system call table*/
void* syscalls_tbl[SYSCALL_NR] = 
	{
		do_write,
		do_read, 
		do_malloc,
		do_exit,
		do_sleep
	};

/* 
 * The handler for the int 80h.
 * The syscall number is in %rax. 
 */
void syscall_handler(void)
{
	/* Pushing dummy data to maintain uniformity with other handlers */
	__asm__ __volatile__(
			     "pushq $0x0\n\t" 
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
			     "pushq %r15\n\t");
	u64int rax;
	__asm__ __volatile__ ("movq %%rax, %0":"=r"(rax));
	if (rax >= SYSCALL_NR)
		return;
	void *location = syscalls_tbl[rax];
	u64int ret;
	__asm__ __volatile__ ("callq *%0\n\t  " : "=a" (ret) :  "r" (location));
	__asm__ __volatile__("popq %r15\n\t"
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
			     "addq $8, %rsp\n\t" /* Discard dummy data */
			     );
	__asm__ ("iretq\n\t");
}

