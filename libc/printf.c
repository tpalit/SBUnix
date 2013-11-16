#include<sys/syscall.h>

int printf(const char *format, ...) {
	/* Store the pointer in the rdx register. */
	__asm__ __volatile__("movq %[s], %%rdx\n\t"
			     :
			     :[s]"m"(format));
	/* Store the system call index in the rax register. */
	__asm__ __volatile__("movq $0, %rax\n\t");
	__asm__("int $0x80\n\t");
	return 0;
}
