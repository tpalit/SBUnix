#include<stdio.h>
#include<unistd.h>
char buffer[100];

char* argp[] = {"Tapti", "Palit", "Ganesh"};
char* envp[] = {"Hello", "World"};

int main(void)
{
	while(1) {
		printf(" \n");
		printf("sh$ ");
		scanf("%s", buffer);
		printf(" \n");
		if(fork()==0){
			execvpe(buffer, argp, envp);
		} else {
			wait();
		}
	}
	return 0;
}
