/**
 * Copyright (c) 2013 by Tapti Palit, Kaustubh Gharpure. All rights reserved.
 */

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>

int main(int argc, char* argv[], char* envp[]) 
{
	int sleep_time = 60; /* default */
	if(!isnullstring(argv[1])) {
		sleep_time = atoi(argv[1])*60;
		sleep(sleep_time);
	}
	return 0;
}
