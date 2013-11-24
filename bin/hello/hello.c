#include <stdio.h>
#include <stdlib.h>

int k;
int main(int argc, char* argv[], char* envp[])
{
	/*
	long int* ptr = (long int*)malloc(sizeof(long int));
	*ptr = 0x200UL;
	printf("Successfully malloced!");
	while(1);
	return 0;
	*/
	int i = 0;
	int k = 0;
	while(1) {
		i++;
		if(i%100000 == 0) {
			printf("Palit");
			k++;
			if(k>30) break;
		}
	}
	int* ptr = (int*)malloc(sizeof(int));
	*ptr = 90;
	printf("Done hello.c");
	return 0;
	
}
