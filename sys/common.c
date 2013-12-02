#include<common.h>
#include<stdio.h>
/*
 * This file will contain the common routines which 
 * will be required all over the world. 
 *
 * Copyright (c) 2013 by Tapti Palit. All rights reserved.
 */


/*
 * A custom outb routine.
 */
void outb(u16int port, u8int val)
{
	__asm__ __volatile__("outb %0, %1"
		     : : "a"(val), "Nd"(port));
}

void panic(char* msg)
{
	kprintf("\nKernel panic: %s\n", msg);
	while(1);
}

void dump_regs(void)
{
	volatile u64int rsp, rbp, rsi, rdi, rax, rbx, rcx, rdx, rip, rflags;
	volatile u64int register cr0, cr2, cr3, cr4;
	__asm__ __volatile__(
			     "movq %%rsp, %[rsp]\n\t"
			     "movq %%rbp, %[rbp]\n\t"
			     "movq %%rsi, %[rsi]\n\t"
			     "movq %%rdi, %[rdi]\n\t"
			     "movq %%rax, %[rax]\n\t"
			     "movq %%rbx, %[rbx]\n\t"
			     "movq %%rcx, %[rcx]\n\t"
			     "movq %%rdx, %[rdx]\n\t"
			     "pushfq\n\t"
			     "popq %[rflags]\n\t"
			     :[rsp]"=m"(rsp),
			      [rbp]"=m"(rbp),
			      [rsi]"=m"(rsi),
			      [rdi]"=m"(rdi),
			      [rax]"=m"(rax),
			      [rbx]"=m"(rbx),
			      [rcx]"=m"(rcx),
			      [rdx]"=m"(rdx),
			      [rflags]"=m"(rflags));
	__asm__ __volatile__(
			     "leaq (%%rip), %[rip]\n\t"
			     "movq %%cr0, %[cr0]\n\t"
			     "movq %%cr2, %[cr2]\n\t"
			     "movq %%cr3, %[cr3]\n\t"
			     "movq %%cr4, %[cr4]\n\t"
			     :
			     [rip]"=r"(rip),
			     [cr0]"=r"(cr0), 
			     [cr2]"=r"(cr2),
			     [cr3]"=r"(cr3),
			     [cr4]"=r"(cr4));
	kprintf("rsp = %p, rbp = %p, rsi = %p,\n"
		"rdi = %p, rax = %p, \n"
		"rbx = %p, rcx = %p, rflags = %p,\n"
		"rip = %p, cr0 = %p, \n"
		"cr2 = %p, cr3 = %p, cr4 = %p\n", rsp, rbp, rsi, rdi, rax, 
		rbx, rcx, rflags, rip, cr0, cr2, cr3, cr4);

}
