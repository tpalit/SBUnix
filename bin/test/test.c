#include <stdio.h>

int k;
int main(int argc, char* argv[]) 
{
	int i = 0;
	int k = 0;
	while(1) {
		i++;
		if(i%100000 == 0) {
			printf("Hello");
			k++;
			if(k>30) break;
		}
	}
	printf("Done test.c");
	while(1);
	return 0;
}
