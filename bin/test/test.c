#include<stdio.h>
#include<stdarg.h>
#include<unistd.h>

char* argp[] = {"Tapti", "Palit", "Ganesh"};
char* envp[] = {"Hello", "World"};

int main(void)
{
	execvpe("bin/hey", argp, envp);
	return 0;
}
