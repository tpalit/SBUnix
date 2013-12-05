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
		char arr[1024]; /* Huge string */
		struct node * next;
	};
	while(1) {
		struct node * ptr = (struct node *)malloc(sizeof(struct node));
		printf("Allocated memory ... ");
		ptr->i=10;
		strcpy(ptr->arr,"hello");
		ptr->next=NULL;
	}
	return 0;
}
