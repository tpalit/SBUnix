#include<stdio.h>
#include<unistd.h>

long int k = -2;
int main(void)
{
	if(fork()){
		k++;
		printf("In parent\n");
		wait();
		printf("Done waiting\n");
	} else {
		int i = 0;
		for(i=0; i<10; i++){
			printf("In child\n");
		}
       		execvpe("bin/hey", NULL, NULL);
	}
	return 0;
}
