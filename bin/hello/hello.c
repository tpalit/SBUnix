#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

int main(int argc, char* argv[], char* envp[])
{
	int k = 0;
	while(k<30) {
		printf("Hello...");
		sleep(1);
		k++;
	}
	printf("Done hello.c");
	return 0;
	
}
