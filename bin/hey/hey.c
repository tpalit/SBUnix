#include<stdio.h>
#include<stdlib.h>

int main(int argc, char* argv[], char* envp[])
{
	printf("Executing hey!");
	char* p = (char*)malloc(sizeof(char));
	*p = 'A';
	printf("%c", *p);
	printf("Value = %s", argv);
	return 0;
}
