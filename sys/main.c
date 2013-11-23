/*
 * The main file. The execution of the kernel begins from here.
 *
 * Copyright (c) by Tapti Palit. All rights reserved.
 *
 */

#include<defs.h>
#include<stdio.h>
#include<common.h>
#include<sys/vm_mgr.h>
#include<sys/pm_mgr.h>
#include<sys/kmalloc.h>
#include<sys/proc_mgr.h>
#include<sys/elf64.h>

#define LOW_MEM 0
#define HI_MEM 1

void clear_terminal(void);
void init_timer(int);

#define INITIAL_STACK_SIZE 4096
char stack[INITIAL_STACK_SIZE];
uint32_t* loader_stack;
extern char kernmem, physbase;

extern void reset_terminal_cursor();
extern void clear_terminal();
extern void init_timer(int);
extern void initialize_gdt();
extern void initialize_idt();
extern void initialize_tss();

extern void switch_to_user_mode(void);

/* These should NOT be used directly outside of the first initialization*/
pml4_e pml_entries[512] __attribute__((aligned(0x1000)));
pdp_e pdp_entries[512] __attribute__((aligned(0x1000))); // First table of PDP
pd_e pd_entries[512] __attribute__((aligned(0x1000))); // First table of PD
pt_e pt_entries[512] __attribute__((aligned(0x1000))); // First table of PT
cr3_reg cr3_register;
void start(uint32_t* modulep, void* physbase, void* physfree)
{
	struct smap_t {
		uint64_t base, length;
		uint32_t type;
	}__attribute__((packed)) *smap;
	int i=0;
	struct smap_t smap_mem_regs[2]; 
	clear_terminal();
	while(modulep[0] != 0x9001) modulep += modulep[1]+2;
	for(smap = (struct smap_t*)(modulep+2); 
	    smap < (struct smap_t*)((char*)modulep+modulep[1]+2*4); 
	    ++smap) {
		if (smap->type == 1 /* memory */ && smap->length != 0) {
			smap_mem_regs[i].base = smap->base;
			smap_mem_regs[i].length = smap->length;
			smap_mem_regs[i++].type = smap->type;
		}
	}
	
	init_pg_frames(smap_mem_regs[HI_MEM].base, 
		       smap_mem_regs[HI_MEM].base+smap_mem_regs[1].length, 
		       &physbase, 
		       &physfree);
	init_pg_dir_pages(pml_entries);
	map_phys_vir_range((u64int)physbase, (u64int)physfree, (u64int)(KERN_VIR_START+physbase));	
	create_cr3_reg(&cr3_register, (u64int)pml_entries-KERN_VIR_START, 0x00, 0x00);
	__asm__ __volatile__(
			     "movq %0, %%cr3\n\t"
			     ::"a"(cr3_register));
	/* Setup the stack again. */
	__asm__ __volatile__("movq %0, %%rbp" : :"a"(&stack[0]));
	__asm__ __volatile__("movq %0, %%rsp" : :"a"(&stack[INITIAL_STACK_SIZE]));
       	initialize_tss();
	make_process_from_elf("bin/hi");
      	make_process_from_elf("bin/hello");
	make_process_from_elf("bin/test");
	__asm__("sti\n\t");
	while(1);
}


void boot(void)
{
	// note: function changes rsp, local stack variables can't be practically used
	volatile register char *rsp asm ("rsp");
	char* temp1, *temp2;
	loader_stack = (uint32_t*)rsp;
	rsp = &stack[INITIAL_STACK_SIZE];
	initialize_gdt();
	__asm__("cli\n\t");
       	initialize_idt();
	init_timer(10);
	start(
		(uint32_t*)((char*)(uint64_t)loader_stack[3] + (uint64_t)&kernmem - (uint64_t)&physbase),
		&physbase,
		(void*)(uint64_t)loader_stack[4]
	);
	for(
		temp1 = "!!!!! start() returned !!!!!", temp2 = (char*)0xb8000;
		*temp1;
		temp1 += 1, temp2 += 2
	) *temp2 = *temp1;
	while(1);
}


