/**
 * Copyright (c) 2013 by Tapti Palit, Kaustubh Gharpure. All rights reserved.
 */

#include<syscall.h>
#include<stdio.h>
#include<stdarg.h>
#include<common.h>
#include<sys/terminal.h>
#include<sys/kstring.h>

char scandata[100];
void cleanbuffer(){
    int i;
    for(i =0;i<100;i++) {
        scandata[i]=NULL; // clearing the buffer
    }
}

int getbuffer(char *format,int fd,int size) {
	register volatile u64int ret_val = 0;
	/* Store the pointer in the rdx register. */
	__asm__ __volatile__("movq %[s], %%rdx\n\t"
			     :
			     :[s]"m"(format));
	__asm__ __volatile__("movq %[i], %%r8\n\t"
			     :
			     :[i]"m"(fd));
	__asm__ __volatile__("movq %[size], %%r9\n\t"
			     :
			     :[size]"m"(size));
	/* Store the system call index in the rax register. */
	__asm__ __volatile__("movq $1, %rax\n\t");
	__asm__("int $0x80\n\t");
	/* The return value is also in rax register. */
	__asm__("movq %%rax, %[retVal]\n\t":[retVal]"=r"(ret_val));
	return ret_val;
}
int str2dec(char* str)
{
   // if(!str)
     //  kprintf("Enter valid string");


    int number = 0;
    char* p = str;

    while((*p >= '0') && (*p <= '9'))
    {
        number = number * 10 + (*p - '0');
        p++;
    } 
    return number;
}
char xtod(char c) {
    if (c>='0' && c<='9') return c-'0';
    if (c>='A' && c<='F') return c-'A'+10;
    if (c>='a' && c<='f') return c-'a'+10;
    return c=0;        // not Hex digit
}

int HextoDec(char *hex, int l)
{
    if (*hex==0) return(l);
    return HextoDec(hex+1, l*16+xtod(*hex)); // hex+1?
}

int str2hex(char *hex)      // hex string to integer
{
    return HextoDec(hex,0);
}
static int scan(char **in, const char *fmt, va_list args) {
    int converted = 0;
    const char * p;
    cleanbuffer();
    for(p=fmt;*p!='\0';p++){
        if (*p == '%') {
            p++;
            switch (*p) {
                case 's': {
                              char *dst = va_arg(args, char*);
                              getbuffer(scandata,std_in,0);
                              strcpy(dst,scandata);
                              ++converted;
                          }
                          continue;
               case 'c': {
                              int dst;
                              getbuffer(scandata,std_in,0);
                              dst=(int)scandata[0];
                              *va_arg(args, char*) = dst;
                              ++converted;
                          }
                          continue;
                 case 'd': {
                              int dst=0;
                              getbuffer(scandata,std_in,0);
                              dst=str2dec(scandata);
                              *va_arg(args, int*) = dst;
                              ++converted;
                          }
                          continue;
             
                case 'x': {
                              int dst=0;
                              getbuffer(scandata,std_in,0);
                              dst=str2hex(scandata);
                              *va_arg(args, int*) = dst;
                              ++converted;
                          }
                          continue;
            }
        } else {
            if (*fmt++ != *(*in)++) {
                return converted;
            }
        }
    }
   // while(1);
    return converted;
}
int scanf(const char *format, ...) {
    va_list args;
    int rv;

    va_start(args, format);
    rv = scan(0, format, args);
    va_end(args);

    return rv;
}

