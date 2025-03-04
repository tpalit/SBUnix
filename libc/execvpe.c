/**
 * Copyright (c) 2013 by Tapti Palit, Kaustubh Gharpure. All rights reserved.
 */

#include<syscall.h>
#include<common.h>

u32int execvpe(char* path, char* argv[], char* envp[]) {
	register volatile u64int ret_val = 0;
	/* Store the system call index in the rax register. */
	/* Store the pointer in the rdi register. */
	__asm__ __volatile__("movq %[s], %%rdi\n\t"
			     "movq %[argv], %%rsi\n\t"
			     "movq %[envp], %%rdx\n\t"
			     :
			     :[s]"m"(path), [argv]"m"(argv), [envp]"m"(envp));
	__asm__ __volatile__("movq $11, %rax\n\t");
	__asm__("int $0x80\n\t");
	/* The return value is also in rax register. */
	__asm__("movq %%rax, %[retVal]\n\t":[retVal]"=r"(ret_val));
	return ret_val;
}

