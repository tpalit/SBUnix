#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
char buffer[100];
char path[50];

int main(int argc, char* argv[], char* envp[])
{
    char command[20]={NULL},runcommand[22]="bin",token1[10]={NULL},token2[10]={NULL};
	char** str_ptr = (char**)malloc(sizeof(char*));
	char* str = (char*)malloc(10*sizeof(char));
	strcpy(str, "dummy");
	str_ptr = &str;
	while(1) {
		printf(" \n");
		printf("User@Sbunix %s$",path);
		scanf("%s", buffer);
                trimspaces(buffer);
                tokanize(buffer,token1,token2);
                trimspaces(token2); //this token 2 will be used as the arguments
                strcpy(command,token1);
                if(strcmp(command,"cd")){
                addpath(path,token2);
                }
                if(strcmp(command,"cd")||strcmp(command,"ls")||strcmp(command,"ps")||strcmp(command,"ulimit"))
                {
                    addpath(runcommand,command);
                }
                else if(command[0]=='.'&&command[1]=='/')
                {
                    addpath(runcommand,command+2);
                }
		printf(" \n");
		if(fork()==0){
			execvpe(runcommand, str_ptr, NULL);// I dont know how we get return values from the binaries we call but cd should return file descriptor
		} else {
			wait();
		}
	}
	return 0;
}
