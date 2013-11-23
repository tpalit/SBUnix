#include<syscall.h>
#include<common.h>

void exit(int status) {
	/* Store the system call index in the rax register. */
	__asm__ __volatile__("movq $3, %rax\n\t");
	__asm__("int $0x80\n\t");
}
