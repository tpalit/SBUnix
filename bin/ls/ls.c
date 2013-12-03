
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<file.h>
#include<sys/kstring.h>
#define buffsize 15
int main(int argc, char* argv[], char* envp[])
{
    /* For LS we need to pass the dir descriptor as an argument 
     * the shell will get that dir descriptor from the cd 
     * for now I am opening a descriptor here for test purpose*/
    int i;
    int f = opendir("filesystem");
    char buff[buffsize];
    readdir(buff,f);
    printf("\n%s",buff);
    while(buff[0]!=NULL)
    {
        for(i=0;i<buffsize;i++)
            buff[i]=NULL;
        readdir(buff,f);
        printf("\n%s",buff);
    }
    return 0;
}
