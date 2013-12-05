#include<syscall.h>
#include<common.h>

u32int waitpid(u32int child_pid) {
	register volatile u64int ret_val = 0;
	/* Store the pointer in the rdx register. */
	__asm__ __volatile__("movq %[pid], %%rdi\n\t"
			     :
			     :[pid]"m"(child_pid));
	/* Store the system call index in the rax register. */
	__asm__ __volatile__("movq $13, %rax\n\t");
	__asm__("int $0x80\n\t");
	/* The return value is also in rax register. */
	__asm__("movq %%rax, %[retVal]\n\t":[retVal]"=r"(ret_val));
	return ret_val;
}

