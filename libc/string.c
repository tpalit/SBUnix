
#include<string.h>
#include<common.h>

int strcmp(char* str1, char* str2)
{
	int i=0,j=0,k=0;
	while (str1[i]!='\0') {
		i=i+1;
	}
	while (str2[j]!='\0') {
		j=j+1;
	}
	if(i!=j) {
		return 0;
	} else	{
		for(k=0;k<=i;k++) {
			if(str1[k]!=str2[k]) {
				return 0;
			}
		}
		return 1;
	}
}

void strcpy(char *target, char *source)
{
	while(*source)
		{
			*target = *source;
			source++;
			target++;
		}
	*target = '\0';
}

void* memcpy(void* dest, void* src, int count)
{
	char* dst1 = (char*)dest;
        char* src1 = (char*)src;
        while (count--) {
		*dst1++ = *src1++;
        }
        return dest;
}
void trimpath(char *t1)
{
    while(*t1)
        t1++;
    while(*t1!='/')
    {
        *t1=NULL;
        t1--;
    }
    *t1=NULL;

}
void form_full_path(char* full_path, char * t1,char * t2)
{
    while(*t1) {
	    *full_path++ = *t1++;
    }
    *full_path++='/';

    while(*t2) {
	    *full_path++ = *t2++;
    }
    *full_path='\0';
}

void trimspaces(char * t1){
    char * temp;
    temp=t1;
    while(*temp==' '){
        temp++;
    }
    while(*temp)
    {*t1=*temp;
        t1++;
        temp++;
    }
    while(*t1==' ')
    {
        *t1=NULL;
        t1--;
    }
}
void tokanize(char *t1,char *t2, char *t3){

    while(*t1!=' '&&*t1){
        *t2=*t1;
        t1++;
        t2++;
    }
    *t2='\0';
    t1++;
    while(*t1){
        *t3=*t1;
        t1++;
        t3++;
    }
    *t3='\0';
}
