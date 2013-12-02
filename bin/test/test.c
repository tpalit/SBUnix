#include<stdio.h>
#include<unistd.h>

int main(void)
{
	execvpe("bin/hey", NULL, NULL);
	return 0;
}
