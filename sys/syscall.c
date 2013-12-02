/**
 * This file contains the functionalities to register and invoke system calls.
 */


#include<common.h>
#include<syscall.h>
#include<sys/proc_mgr.h>
#include<sys/kstring.h>
#include<stdio.h>
#include<sys/fork.h>
#include<sys/dir.h>
#include<sys/elf64.h>
#include<sys/tarfs.h>
#define SYSCALL_NR 15


extern task_struct* CURRENT_TASK;

/* Need to save the rsp. Would've been easier if we had pushed and popped rsp */
u64int syscalling_task_rsp; /* kernel stack */
u64int syscall_ret_address;
u64int syscalling_task_user_rsp; /* the user stack */

/* These will get invoked in kernel mode. */
int do_write(char* s)
{
    kprintf(s);
    return 0;
}

int do_read(char* s,int i,int bsize)
{
    char * ptr=NULL;
    int o = 0;
    if(i==0){
        for(o =0 ;o<100;o++) {
            STDIN[o]=NULL; // clearing the buffer
        }
        __asm__ __volatile__("sti\n\t");
        writebuff=0;
        while(writebuff==0){
            if(writebuff!=0)
                break;
        }
        kprintf("finally %s",STDIN);
        kstrcpy(s,STDIN);
    }
    else{
        ptr=read_ptr(i);
        if(*ptr!=NULL)
            if(bsize!=0)
                kstrcpysz(s,ptr,bsize);
            else
                kstrcpy(s,ptr);

        else
            kprintf("\nfile not open");
    }
    return 0;
}

int do_open(char *s)
{
    int result;
    result=find_file(s);
    return result;
}
int do_close(int i)
{
    int result;
    result= close_fd(i);
    return result;
}
int do_opendir(char *s){
    int result;
    result = find_dir(s);
    return result;
}
int do_readdir(char *s,int dird)
{
    int c,k,byte_size,size;
    dir_ptr *i;
    posix_header_ustar *ptr;
    i=read_dir(dird);
    c=i->count;
    ptr=i->dir_entry;
    if(c==0){
        trim(i->dir_path,ptr->name,s);
        i->count++;
        return 0;
    }
    else{
        for (k=0;k<c;k++){
            byte_size = tarfs_atoi(ptr->size,8) + sizeof(posix_header_ustar); // size in bytes
            size = byte_size/sizeof(posix_header_ustar);
            if (byte_size%sizeof(posix_header_ustar) != 0)
                size++;
            ptr = ptr + size;
        }
        if(kstrcmpsz(i->dir_path,ptr->name))
        {
            trim(i->dir_path,ptr->name,s);
            i->count++;
            return 0;
        }

    }
    return -1;
}
int do_closedir(int dird){
    return close_dird(dird);
}
int do_malloc(u32int mem_size)
{
    u64int ret_ptr;
    vm_struct* vma_ptr = CURRENT_TASK->vm_head;
    while (vma_ptr != NULL){
        if(vma_ptr->vm_type == HEAP_VMA){
            break;
        }
        vma_ptr = vma_ptr->vm_next;
    }
    if(vma_ptr == NULL){
        panic("A process with no heap found!");
    }
    ret_ptr = vma_ptr->vm_end;
    vma_ptr->vm_end+=mem_size;
    return ret_ptr;
}

void do_exit(void)
{
    exit();
}

void do_sleep(u32int sleep_time)
{
    CURRENT_TASK->rsp_register = syscalling_task_rsp;
    sleep(sleep_time);
}

int do_exec(char* file, char* argv[], char* envp[])
{
	overlay_task(file, CURRENT_TASK);
	return SUCCESS;
}

/* Set up the system call table*/
void* syscalls_tbl[SYSCALL_NR] =
{
    do_write, /*   0 */
    do_read,  /*   1 */
    do_malloc,/*   2 */
    do_exit,  /*   3 */
    do_sleep, /*   4 */
    do_fork,   /*   5 */
    do_open,  /*   6  */
    do_close,  /*  7  */
    do_opendir, /*  8  */
    do_closedir, /*  9  */
    do_readdir,  /*  10  */
    do_exec  /* 11 */
};

/*
 * The handler for the int 80h.
 */
void syscall_handler(void)
{
    /*
     * The syscall number is in rax.
     * The result will also be in the rax.
     * So don't pop back rax.
     */
    __asm__ __volatile__(
            "pushq %rax\n\t"
            "pushq %rbx\n\t"
            "pushq %rcx\n\t"
            "pushq %rdx\n\t"
            "pushq %rbp\n\t"
            "pushq %rsi\n\t"
            "pushq %rdi\n\t"
            "pushq %r8\n\t"
            "pushq %r9\n\t"
            "pushq %r10\n\t"
            "pushq %r11\n\t"
            "pushq %r12\n\t"
            "pushq %r13\n\t"
            "pushq %r14\n\t"
            "pushq %r15\n\t"
	    "pushq %rdi\n\t"
	    "pushq %rsi\n\t"
	    "pushq %rdx\n\t");
    u64int rax;
    __asm__ __volatile__ ("movq %%rax, %0":"=r"(rax));
    __asm__ __volatile__("movq %%rsp, %[old_rsp]\n\t": [old_rsp] "=r"(syscalling_task_rsp));
    __asm__ __volatile__("movq 144(%%rsp), %[ret_addr]\n\t":[ret_addr] "=r"(syscall_ret_address) );
    __asm__ __volatile__("movq 168(%%rsp), %[ustck_addr]\n\t":[ustck_addr] "=r"(syscalling_task_user_rsp) );
    /*
    __asm__ __volatile__ ("movq %%rax, %[rax]\n\t"
			  "movq %%rsp, %[old_rsp]\n\t"
			  "movq 120(%%rsp), %[ret_addr]\n\t"
			  "movq 144(%%rsp), %[ustck_addr]\n\t"
			  :[rax]"=r"(rax), [old_rsp] "=r"(syscalling_task_rsp), [ret_addr] "=r"(syscall_ret_address),
			  [ustck_addr] "=r"(syscalling_task_user_rsp)
			  :
			  :"rdi","rsi", "rdx" );
    */
    if (rax >= SYSCALL_NR)
        return;
    void *location = syscalls_tbl[rax];
    u64int ret;
    __asm__ __volatile__("popq %%rdx\n\t"
			 "popq %%rsi\n\t"
			 "popq %%rdi\n\t"
			 "callq *%0\n\t  " : "=a" (ret) :  "r" (location));
    __asm__ __volatile__("popq %r15\n\t"
            "popq %r14\n\t"
            "popq %r13\n\t"
            "popq %r12\n\t"
            "popq %r11\n\t"
            "popq %r10\n\t"
            "popq %r9\n\t"
            "popq %r8\n\t"
            "popq %rdi\n\t"
            "popq %rsi\n\t"
            "popq %rbp\n\t"
            "popq %rdx\n\t"
            "popq %rcx\n\t"
            "popq %rbx\n\t"
            "addq $8, %rsp\n\t" /* Don't want to overwrite the rax register */
            );
    __asm__ ("iretq\n\t");
}

