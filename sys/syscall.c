/**
 * This file contains the functionalities to register and invoke system calls.
 */


#include<common.h>
#include<sys/syscall.h>
#include<sys/proc_mgr.h>
#include<sys/kstring.h>
#include<stdio.h>

#define SYSCALL_NR 10

extern task_struct* CURRENT_TASK;

/* These will get invoked in kernel mode. */
int do_write(char* s)
{
	kprintf("in do_write\n");
	while(*s != '\0'){
		putchar(*s++);
	}
	return 0;
}

int do_read(char* s)
{
	return 0;
}

int do_malloc(u32int mem_size)
{
	u64int ret_ptr;
	/* The task that was scheduled last is requesting memory. 
	 * @TODO - Check if this is assumption right.
	 * Need to expand its heap. 
	 */
	kprintf("inside do_malloc for %d", mem_size);
	vm_struct* vma_ptr = CURRENT_TASK->vm_head;
        while (vma_ptr != NULL){
		if (kstrcmp(vma_ptr->vm_type, "HEAP")){
			break;
		}
		vma_ptr = vma_ptr->vm_next;
	}
	if(vma_ptr == NULL){
		panic("A process with no heap found!");
	}
	ret_ptr = vma_ptr->vm_end;
	vma_ptr->vm_end+=mem_size;
	kprintf("ret_ptr = %p", ret_ptr);
	return ret_ptr;
}

/* Set up the system call table*/
void* syscalls_tbl[SYSCALL_NR] = 
	{
		do_write,
		do_read, 
		do_malloc
	};

/* The handler for the int 80h */
void syscall_handler(void)
{
	u64int rax;
	__asm__ __volatile__ ("movq %%rax, %0":"=r"(rax));
	if (rax >= SYSCALL_NR)
		return;
	void *location = syscalls_tbl[rax];
	u64int ret;
	__asm__ __volatile__ ("callq *%0\n\t  " : "=a" (ret) :  "r" (location));
	__asm__ ("iretq\n\t");
}

