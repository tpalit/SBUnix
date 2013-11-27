#include<syscall.h>
#include<common.h>

u32int execvpe(char* path, char* argv[], char* envp[]) {
	register volatile u64int ret_val = 0;
	/* Store the system call index in the rax register. */
	/* Store the pointer in the rdx register. */
	__asm__ __volatile__("movq %[s], %%rdx\n\t"
			     :
			     :[s]"m"(path));
	__asm__ __volatile__("movq $6, %rax\n\t");
	__asm__("int $0x80\n\t");
	/* The return value is also in rax register. */
	__asm__("movq %%rax, %[retVal]\n\t":[retVal]"=r"(ret_val));
	return ret_val;
}

