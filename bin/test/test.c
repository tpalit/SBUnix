#include<stdio.h>
#include<unistd.h>

char* argp[] = {"Tapti", "Palit", "Ganesh"};
char* envp[] = {"Hello", "World"};

int main(void)
{
	execvpe("bin/hey", argp, envp);
	return 0;
}
