#include<stdio.h>
#include<unistd.h>
char buffer[100];

int main(void)
{
	while(1) {
		printf(" \n");
		printf("sh$ ");
		scanf("%s", buffer);
		printf(" \n");
		if(fork()==0){
			execvpe(buffer, NULL, NULL);
		} else {
			wait();
		}
	}
	return 0;
}
