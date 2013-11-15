#include<sys/syscall.h>

/* The system call interfaces. The libc will invoke these. */
int sys_write(char* s)
{
	/* Store the pointer in the rdx register. */
	__asm__ __volatile__("movq %[s], %%rdx\n\t"
			     :
			     :[s]"m"(s));
	/* Store the system call index in the rax register. */
	__asm__ __volatile__("movq $0, %rax\n\t");
	__asm__("int $0x80\n\t");
	return 0;
}

int sys_read(char* s)
{
	return 0;
}

int printf(const char *format, ...) {
	char* test = "Hello World!";
	sys_write(test);
	return 0;
}
