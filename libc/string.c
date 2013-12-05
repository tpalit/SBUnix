/**
 * Copyright (c) 2013 by Tapti Palit, Kaustubh Gharpure. All rights reserved.
 */
#include<string.h>
#include<common.h>
#include<stdlib.h>
#include<stdio.h>

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

int strlen(char* str)
{
	int length = 0;
	while (*str++ != '\0'){
		length++;
	}
	return length;
}

int isnullstring(char* str)
{
	if(str == NULL){
		return 1;
	}
	if(*str == '\0') {
		return 1;
	} else {
		return 0;
	}
}


/**
 * Tokenize the string. 
 */
char** strtok(char* str)
{
	char** result = (char**)malloc(MAX_TOKENS*sizeof(char*));
	char temp[32];
	int k = 0;
	for (k=0; k<32;k++){
		temp[k] = '\0';
	}
	int i = 0, j=0;
	int processed = 0;
	do{
		if ((*str == ' ' || *str == '\0') && strlen(temp)>0){
			temp[j] = '\0';
			/* Add to the result */
			*(result+i) = (char*)malloc((strlen(temp)+1)*sizeof(char));
			strcpy(*(result+i), temp);
			i++;
			/* Clear the buffer */
			for(k=0;k<32;k++) {
				temp[k] = '\0';
			}
			j = 0;
			if(*str == '\0'){
				processed = 1;
				break;
			}
			str++;
		} else {
			temp[j++] = *str++;
		}
	} while(*str != '\0');
	if(!processed) {
		temp[j] = '\0';
		*(result+i) = (char*)malloc(strlen(temp)*sizeof(char));
		strcpy(*(result+i), temp);
	}
	return result;
}

int atoi(char* s){
	int value = 0;
	while(*s != '\0'){
		value = value*10+(*s-'0');
		s++;
	}
	return value;
}
