#include<stdio.h>
#include<unistd.h>

int main(void)
{
	int k = 0;
	while(k<5) {
		printf("Hi!!");
		sleep(1);
		k++;
	}
	printf("Done hi.c");
	return 0;
	/*
	int i = 0;
	int k = 0;
	while(1) {
		i++;
		if(i%100000 == 0) {
      			printf("Tapti");
			sleep(50);
			k++;
			if(k>30) break;
		}
	}
	printf("Done hi.c");
	*/
}
