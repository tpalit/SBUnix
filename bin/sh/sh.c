#include<stdio.h>
#include<unistd.h>
int main(void)
{
	volatile int cond = 1;
	while(cond) {
		//		printf("Going to fork again!");
		if(fork()){
			printf("In parent\n");
			wait();
			printf("Done waiting\n");
		} else {
			execvpe("bin/hey", NULL, NULL);
		}
	}
	return 0;
}
