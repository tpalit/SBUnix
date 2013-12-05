#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

int main(int argc, char* argv[], char* envp[])
{
	int* dangerous_ptr = NULL;
	*dangerous_ptr = 70;
	return 0;
}
