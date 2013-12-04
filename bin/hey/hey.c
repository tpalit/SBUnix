#include<stdio.h>
#include<unistd.h>
#include<stdarg.h>

int main(int argc, char* argv[], char* envp[])
{
	while(1){
		sleep(10);
		printf("hey! ");
	}
	return 0;
}
