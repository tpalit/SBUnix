
#include<syscall.h>
#include<common.h>
#include<stdio.h>
#include<sys/kstring.h>

int getfilename(char *format,u64int fd) {
    register volatile u64int ret_val = 0;
    /* Store the pointer in the rdx register. */
    __asm__ __volatile__("movq %[s], %%rdi\n\t"
            :
            :[s]"m"(format)
            :"rdi");
    __asm__ __volatile__("movq %[i], %%rsi\n\t"
            :
            :[i]"m"(fd)
            :"rsi");
    /* Store the system call index in the rax register. */
    __asm__ __volatile__("movq $10, %rax\n\t");
    __asm__("int $0x80\n\t");
    /* The return value is also in rax register. */
    __asm__("movq %%rax, %[retVal]\n\t":[retVal]"=r"(ret_val));
    return ret_val;
}

int readdir(char *target,int fd){
    char buffer[100]={NULL};
    getfilename(buffer,fd);
    strcpy(target,buffer);
    return 0;
}
