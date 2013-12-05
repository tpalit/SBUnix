#include<stdio.h>
#include<unistd.h>
#include<stdarg.h>

int main(int argc, char* argv[], char* envp[])
{
	printf("Such is life!\n");
	printf("My pid is %d", getpid());
	return 0;
}
