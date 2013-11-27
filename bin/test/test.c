#include<stdio.h>
#include<unistd.h>

int main(void)
{
	execvpe("bin/hello", NULL, NULL);
	return 0;
}
