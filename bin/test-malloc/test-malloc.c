
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<file.h>
#include<string.h>

int main(int argc, char* argv[], char* envp[])
{
	struct node{
		int i;
		char arr[20];
		struct node * next;
	};
	printf("Allocating memory for structure ...\n");
	struct node * ptr = (struct node *)malloc(sizeof(struct node));
	ptr->i=10;
	strcpy(ptr->arr,"hello");
	ptr->next=NULL;
	printf("\nWrote int value:%d",ptr->i);
	printf("\nWrote char value:%s",ptr->arr);
	return 0;
}
