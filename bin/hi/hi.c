#include<stdio.h>
#include<unistd.h>

int main(void)
{
	/*
	int k = 0;
	while(k<5) {
		printf("Hi!!");
		sleep(1);
		k++;
	}
	printf("Done hi.c");
	*/
	if(fork()){
		printf("In parent");
	} else {
		printf("In child");
	}
	return 0;
}
