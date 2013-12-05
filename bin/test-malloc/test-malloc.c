
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<file.h>
#include<string.h>
#define BUFFSIZE 50 
int main(int argc, char* argv[], char* envp[])
{
    struct node{
        int i;
        char arr[20];
        struct node * next;
    };
    struct node * ptr = (struct node *)malloc(sizeof(struct node));
    ptr->i=10;
    strcpy(ptr->arr,"hello");
    ptr->next=NULL;
    printf("\nint value:%d",ptr->i);
    printf("\nchar value:%s",ptr->arr);
    return 0;
}
