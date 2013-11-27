#include<stdio.h>
#include<unistd.h>

long int k = -2;
int main(void)
{
	if(fork()){
		k++;
		printf("In parent\n");

	} else {
		printf("In child\n");
		execvpe("bin/hello", NULL, NULL);
	}
	return 0;
}
