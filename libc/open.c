/**
 * Copyright (c) 2013 by Tapti Palit, Kaustubh Gharpure. All rights reserved.
 */

#include<syscall.h>
#include<common.h>
#include<stdio.h>
#include<file.h>
int open(char *fname) {
   //printf("i am here");
   //while(1);
   register volatile u64int ret_val = 0;
    /* Store the pointer in the rdx register. */
    __asm__ __volatile__("movq %[s], %%rdx\n\t"
            :
            :[s]"m"(fname));
    /* Store the system call index in the rax register. */
    __asm__ __volatile__("movq $6, %rax\n\t");
    __asm__("int $0x80\n\t");
    /* The return value is also in rax register. */
    __asm__("movq %%rax, %[retVal]\n\t":[retVal]"=r"(ret_val));
    if(ret_val==0)
        printf("file not found");
    return ret_val;

}
