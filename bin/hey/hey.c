#include<stdio.h>

int main(int argc, char* argv[], char* envp[])
{
	printf("Executing hey!");
	printf("%p", argv);
	printf("Value = %s", argv);
	return 0;
}
