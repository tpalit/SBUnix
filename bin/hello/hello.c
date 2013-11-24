#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

int main(int argc, char* argv[], char* envp[])
{
	int k = 0;
	while(k<5) {
		printf("Hello...");
		sleep(100);
		k++;
	}
	printf("Done hello.c");
	return 0;
	
}
