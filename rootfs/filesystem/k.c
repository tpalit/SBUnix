
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<file.h>
int main(int argc, char* argv[], char* envp[])
{
	//int k = 0;
        //printf("\nplease enter number");
        //scanf("%d",&k);
        //printf("\n the number is %d",k);
        int f = opendir("filesystem");
	printf("\n file des=%d",f);
        char buff[10];
        readdir(buff,f);
        printf("\n %s",buff);
        readdir(buff,f);
        printf("\n %s",buff);
        closedir(f);
        f=opendir("bin/hello");
	printf("\n file des=%d",f);

//	while(k<5) {
//		printf("Hello...");
//		sleep(100);
//		k++;
//	}
	printf("\nDone hello.c");
	return 0;
}
