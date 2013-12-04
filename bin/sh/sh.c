#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/kstring.h>
char buffer[100];


int main(int argc, char* argv[], char* envp[])
{
	char** str_ptr = (char**)malloc(sizeof(char*));
	char* str = (char*)malloc(10*sizeof(char));
	strcpy(str, "dummy");
	str_ptr = &str;
	while(1) {
		printf(" \n");
		printf("sh$ ");
		scanf("%s", buffer);
		printf(" \n");
		if(fork()==0){
			execvpe(buffer, str_ptr, NULL);
		} else {
			wait();
		}
	}
	return 0;
}
