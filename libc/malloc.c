#include<stdio.h>
#include<sys/syscall.h>
#include<common.h>

void* malloc(u64int size) {
	u64int ret_ptr = 0x0;
	/* Store the pointer in the rdx register. */
	__asm__ __volatile__("movq %[s], %%rdx\n\t"
			     :
			     :[s]"m"(size));
	/* Store the system call index in the rax register. */
	__asm__ __volatile__("movq $2, %rax\n\t");
	__asm__("int $0x80\n\t");
	__asm__("movq %%rax, %[ret_ptr]":[ret_ptr]"=m"(ret_ptr));
	return (void*)ret_ptr;
}
