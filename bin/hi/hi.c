#include<stdio.h>
#include<unistd.h>

long int k = -2;
int main(void)
{
	while(1) {
		if(fork()){
			k++;
			printf("In parent\n");
			sleep(1000);
		} else {
			printf("In child\n");
			execvpe("bin/hello", NULL, NULL);
		}
	}
	return 0;
}
