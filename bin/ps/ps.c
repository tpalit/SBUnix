
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<file.h>
#include<sys/kstring.h>
#define buffsize 15
int main(int argc, char* argv[], char* envp[])
{
    char buffer[100];
    getprocinfo(buffer);
    printf(buffer);
    return 0;
}
