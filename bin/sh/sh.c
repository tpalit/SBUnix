#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
char buffer[100];
char cwd[50];
char command[20];
char token1[10];
char token2[10];
char PATH[10] = "bin";
char full_command[40]; 
int main(int argc, char* argv[], char* envp[])
{
	char** str_ptr = (char**)malloc(sizeof(char*));
	char* str = (char*)malloc(10*sizeof(char));
	strcpy(str, "dummy");
	str_ptr = &str;
	while(1) {
		printf("User@Sbunix$");
		scanf("%s", buffer);
		printf("\n");
		if(fork()==0){
			execvpe(buffer, str_ptr, NULL);
		} else {
			wait();
		}
		printf("\n");
	}
	return 0;
}
