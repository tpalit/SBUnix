#include<stdio.h>
#include<unistd.h>
#include<stdarg.h>

int main(int argc, char* argv[], char* envp[])
{
	printf("Such is life!%s\n", argv[0]);
	return 0;
}
