#include<stdio.h>
#include<syscall.h>
#include<common.h>

void* malloc(u64int size) {
	register volatile u64int ret_val = 0;
	/* Store the pointer in the rdx register. */
	__asm__ __volatile__("movq %[s], %%rdx\n\t"
			     :
			     :[s]"m"(size));
	/* Store the system call index in the rax register. */
	__asm__ __volatile__("movq $2, %rax\n\t");
	__asm__("int $0x80\n\t");
	__asm__("movq %%rax, %[ret_ptr]":[ret_ptr]"=r"(ret_val));
	return (void*)ret_val;
}
