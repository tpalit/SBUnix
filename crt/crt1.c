#include <stdlib.h>

void _start(void) {
	int argc = 1;
	char** argv;
	char** envp;
	__asm__ __volatile__("movq %%rsi, %[argv]\n\t"
			     "movq %%rdx, %[envp]\n\t"
			     :[argv]"=m"(argv),
			     [envp]"=m"(envp));
	int res;
	res = main(argc, argv, envp);
	exit(res);
}
