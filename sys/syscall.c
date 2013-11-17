/**
 * This file contains the functionalities to register and invoke system calls.
 */


#include<common.h>
#include<sys/syscall.h>
#include<stdio.h>

#define SYSCALL_NR 10

/* These will get invoked in kernel mode. */
int do_write(char* s)
{
	while(*s != '\0'){
		putchar(*s++);
	}
	return 0;
}

int do_read(char* s)
{
	return 0;
}

/* Set up the system call table*/
void* syscalls_tbl[SYSCALL_NR] = 
	{
		do_write,
		do_read
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

