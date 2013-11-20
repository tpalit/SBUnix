#include<stdio.h>

int main(void)
{
	int i = 0;
	int k = 0;
	while(1) {
		i++;
		if(i%100000 == 0) {
			printf("Tapti");
			k++;
			if(k>30) break;
		}
	}
	printf("Done hi.c");
	while(1);
	return 0;
}
