/**
 * Copyright (c) 2013 by Tapti Palit, Kaustubh Gharpure. All rights reserved.
 */

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<file.h>
#include<sys/kstring.h>
#define BUFFSIZE 50 
int main(int argc, char* argv[], char* envp[])
{
	int i;
	char buff[BUFFSIZE];
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
