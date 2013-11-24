#include<stdio.h>
#include<unistd.h>

int main(int argc, char* argv[]) 
{
	int i = 0;
	while(i<10) {
		i++;
		sleep(20);
		printf("Test");
	}
	printf("Done test.c");
	return 0;
}
