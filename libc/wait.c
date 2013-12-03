#include<syscall.h>
#include<common.h>

u32int wait(u32int time) {
	register volatile u64int ret_val = 0;
	/* Store the pointer in the rdx register. */
	__asm__ __volatile__("movq %[time], %%rdx\n\t"
			     :
			     :[time]"m"(time));
	/* Store the system call index in the rax register. */
	__asm__ __volatile__("movq $12, %rax\n\t");
	__asm__("int $0x80\n\t");
	/* The return value is also in rax register. */
	__asm__("movq %%rax, %[retVal]\n\t":[retVal]"=r"(ret_val));
	return ret_val;
}

