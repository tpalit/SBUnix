/**
 * Copyright (c) 2013 by Tapti Palit, Kaustubh Gharpure. All rights reserved.
 */

#include<sys/kstring.h>
#include<common.h>
#include<sys/kstdio.h>

int kstrlen(char* str)
{
	int length = 0;
	while (*str++ != '\0'){
		length++;
	}
	return length;
}

int kstrcmp(char* str1, char* str2)
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
int kstrcmpsz(char * input,char * string){
    while(* input){
        if(*input!=*string){
            return 0;
        }
        input++;
        string++;
    }
    return 1;
}
void trim(char * t1, char *t2,char * t3){
    while(*t1==*t2){
        t1++;
        t2++;
    }
    while(*t2!='/'&&*t2)
    {
        *t3 = *t2;
        t2++;
        t3++;
    }
    *t3 = '\0';
}
void kstrcpysz(char *target, char *source,int size)
{
    int i;
    kprintf("\nthe size is %d",size);
    for(i=0;i<=size;i++){
        *target = *source;
        source++;
        target++;
    }
    *target = '\0';
}

void kstrcpy(char *target, char *source)
{
	while(*source)
		{
			*target = *source;
			source++;
			target++;
		}
	*target = '\0';
}

void* kmemcpy(void* dest, void* src, int count)
{
	int i = 0;
	u8int* dest_ = (u8int*)dest;
	u8int* src_ = (u8int*)src;
	for(i=0; i<count; i++){
		*(dest_+i) = *(src_+i);
        }
        return dest;
}
void strconcat(char *t1, char *t2)
{
    while(*t1)
    {
        t1++;
    }
    while(*t2)
    {
        *t1=*t2;
        t1++;
        t2++;
    }
    *t1='\0';
}
void num2str(int i, char b[]){
    char const digit[] = "0123456789";
    char* p = b;
    if(i<0){
        *p++ = '-';
        i = -1;
    }
    int shifter = i;
    do{ //Move to where representation ends
        ++p;
        shifter = shifter/10;
    }while(shifter);
    *p = '\0';
    do{ //Move back, inserting digits as u go
        *--p = digit[i%10];
        i = i/10;
    }while(i);

}
void readystr(char * t1){
    while(*t1)
    {
        t1++;
    }
    t1--;
    if(*t1!='/')
    {
        t1++;
        *t1='/';
        t1++;
        *t1='\0';
    }

}
