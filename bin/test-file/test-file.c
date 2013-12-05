
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<file.h>
#include<string.h>
#define BUFFSIZE 50 
int main(int argc, char* argv[], char* envp[])
{
    char buff[BUFFSIZE];
    int fd = open("filesystem/test2.txt");
    printf("\n file descriptor:%d",fd);

    if (fd != 0){
        if(read(buff,fd,10)!=-1)
        {
            printf("\n%s",buff);
        }
        close(fd);

    } else {
        printf("Could not find directory!\n");
    }
    return 0;
}
