/**
 * Copyright (c) 2013 by Tapti Palit, Kaustubh Gharpure. All rights reserved.
 */

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<file.h>
#include<sys/kstring.h>

#define BUFFSIZE 50 

int is_string_empty(char* str)
{
	if(str == NULL){
		return 1;
	}
	if(*str == '\0') {
		return 1;
	} else {
		return 0;
	}
}

int main(int argc, char* argv[], char* envp[])
{
	int i;
	char buff[BUFFSIZE];
	if(NULL == argv || NULL == argv[1] || is_string_empty(argv[1])){
		printf("Please select a directory whose contents to list.\n");
		return 0;
	}
	int fd = opendir(argv[1]);
	if (fd != 0){
		readdir(buff,fd);
		printf("\n%s",buff);
		while(buff[0]!=NULL){
			for(i=0;i<BUFFSIZE;i++)
				buff[i]=NULL;
			readdir(buff,fd);
			printf("\n%s",buff);
		}
	} else {
		printf("Could not find directory!\n");
	}
	return 0;
}
