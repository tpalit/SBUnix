#include<syscall.h>
#include<common.h>
#include<stdio.h>
#include<sys/kstring.h>

int getcontent(char *format,u64int fd,u64int buffsize) {
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
    __asm__ __volatile__("movq %[size], %%rdx\n\t"
            :
            :[size]"m"(buffsize)
            :"rdx");
    /* Store the system call index in the rax register. */
    __asm__ __volatile__("movq $1, %rax\n\t");
    __asm__("int $0x80\n\t");
    /* The return value is also in rax register. */
    __asm__("movq %%rax, %[retVal]\n\t":[retVal]"=r"(ret_val));
    return ret_val;
}

int read(char *target,int fd,int size){
    char buffer[100];
    getcontent(buffer,fd,size);
    strcpy(target,buffer);
    return 0;
}

