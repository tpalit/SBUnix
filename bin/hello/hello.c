#include <stdio.h>
#include <stdlib.h>

int k;
int main(int argc, char* argv[], char* envp[])
{
	//	int i = 0;
	long int* ptr = (long int*)malloc(sizeof(long int));
	*ptr = 0x200UL;
	while(1);
	return 0;
}
