
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<file.h>
#include<sys/kstring.h>
#define buffsize 15
#define pathlength 30
int main(int argc, char* argv[], char* envp[])
{
    /* For CD we need to save every argument that we get till we enconter a ..
     * then on .. we need to trim the arguments back.
     * then we need to make a opendir call for every cd operation 
     * the opendir system call takes care that multiple dir descriptors are not assigned for the same query*/
    char buff[buffsize]={NULL};//this should get the arguments
    char path[pathlength];
    if(buff[0]=='/')
    {
        strcpy(path,buff);
    }
    else if(strcmp(buff,".."))
    {
        trimpath(path);               
    }
    else if(buff[0]!=NULL)
    {
        addpath(path,buff);
    }
    int f = opendir(path);
    return f;
}
