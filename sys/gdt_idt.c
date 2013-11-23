/*
 * This file contains all the code for setting up the gdt, idt, etc.
 * It does some fancy bitwise operations to simplify the setting up
 * of descriptors.
 * And it is a bit of an overkill since most of the fields in the segment 
 * descriptors aren't used at all in long mode, so can be zeroed out 
 * straightaway.
 *
 * Copyright (c) 2013 by Tapti Palit. All rights reserved.
 *
 */

#include<common.h>
#include<sys/desc_tbls.h>
#include<stdio.h>

extern void _flush_gdt(u64int, u64int, u64int);
extern void _flush_idt(u64int);
extern void _flush_tss();

void initialize_gdt();
void initialize_idt();
static void initialize_pic();

extern void syscall_handler(void);
extern void _isr_handler_head_0(void); /* Divide by zero */
extern void isr_handler_body_10(void); /* TSS */
extern void _isr_handler_head_13(void); /* GP */
extern void _isr_handler_head_14(void); /* PF */
extern void _isr_handler_head_32(void); /* Timer */
extern void schedule_on_timer(void); /* Timer */
extern void _isr_handler_head_33(void); /* Keyboard */

static void create_gdt_entry(s32int, u32int, u32int, u8int, u8int);

static void create_idt_entry(s32int, u64int, u16int, u8int, u8int);
			     
static void create_tss_entry(s32int, tss_struct); 

gdt_entry_t gdt_entries[6]; /* 
			       Entries - NULL segment 
			       descriptor and the code and data (kernel and user)
			       segment descriptors and the TSS descriptor (16 bytes). 
			       Note that segmentation is for all practical
			       purposes turned off in long mode.
			    */

tss_struct tss_entry;
idt_entry_t idt_entries[256];

gdtr_pointer gdtr_ptr; 
idtr_pointer idtr_ptr;

void initialize_gdt()
{
	/* 
	 * Most fields are ignored in long mode - AMD64 manual - Pg 137.
	 * Section - Data Segment Descriptors -> Fields ignored in 64-bit.
	 * This is a flat memory model, so base and limit is
	 * not considered. 
	 */
	create_gdt_entry(0, 0, 0, 0, 0); /* The null descriptor */
	create_gdt_entry(1,0x0, 0x0, 0x9A, 0x20); /* For kernel code */
	create_gdt_entry(2, 0x0, 0x0, 0x92, 0x20); /* For kernel data */
	/* num, base, limit, access granularity */
	create_gdt_entry(3, 0x0, 0x0, 0xFA, 0x20); /* For user code */
	create_gdt_entry(4, 0x0, 0x0, 0xF2, 0x20); /* For user data */
	gdtr_ptr.table_limit = sizeof(gdt_entry_t)*10;
	gdtr_ptr.linear_base_addr = (u64int)gdt_entries;
	/* Now, we need to load the address of the gdt_entries into the GDTR */
	/* We will use the LGDT instruction */
	/* And then reload the segment registers */
	_flush_gdt((u64int)&gdtr_ptr, 0x010, 0x08);
}

void initialize_tss()
{
	/* The TSS descriptor is 16 bytes (128 bits), so need to set it up nicely */
	create_tss_entry(5, tss_entry);
	__asm__ __volatile__("movq %%rsp, %[tss_rsp0]\n\t"
			     :[tss_rsp0]"=m"(tss_entry.rsp0));
       	_flush_tss();
}

static void create_gdt_entry(s32int num, 
			     u32int base,
			     u32int limit,
			     u8int access,
			     u8int gran)
{
	gdt_entries[num].base_addr_low = (base & 0xFFFF);
	gdt_entries[num].base_addr_mid = (base >> 16) & 0xFF;
	gdt_entries[num].base_addr_high = (base >> 16) & 0xFF;
	
	gdt_entries[num].seg_limit_low = (limit & 0xFFFF);

	gdt_entries[num].access_misc_bits = access;
	
	gdt_entries[num].gran_misc_bits = (limit >> 16) & 0x0F;
	gdt_entries[num].gran_misc_bits |= gran & 0xF0;

}

static void create_tss_entry(s32int num, tss_struct tss)
{
	sys_segment_descriptor* sd = (sys_segment_descriptor*)&gdt_entries[num];
	sd->sd_lolimit = sizeof(tss_struct)-1;
	sd->sd_lobase = ((u64int)&tss_entry);
	sd->sd_type = 9; // 386 TSS
	sd->sd_dpl = 0;
	sd->sd_p = 1;
	sd->sd_hilimit = 0;
	sd->sd_gran = 0;
	sd->sd_hibase = ((u64int)&tss_entry) >> 24;

}

void initialize_idt()
{
	initialize_pic();
	/* 
	 * Then we create the idt entries for the IRQs.
	 * The ISRs are in the "code segment", so we provide a selector
	 * of 0x08 offset into our GDT.
	 */
	create_idt_entry(32, (u64int)schedule_on_timer, 0x08, 0x0, 0x8E);
	create_idt_entry(33, (u64int)_isr_handler_head_33, 0x08, 0x0, 0x8E);
	
	/*
	 * Now we'll create the idt entries for processor 
	 * faults and exceptions.
	 */
	create_idt_entry(0, (u64int)_isr_handler_head_0, 0x08, 0x0, 0x8E);


	create_idt_entry(10, (u64int)isr_handler_body_10, 0x08, 0x0, 0x8E);

	create_idt_entry(13, (u64int)_isr_handler_head_13, 0x08, 0x0, 0x8E);
	create_idt_entry(14, (u64int)_isr_handler_head_14, 0x08, 0x0, 0x8E);


	create_idt_entry(8, (u64int)_isr_handler_head_14, 0x08, 0x0, 0x8E);
	create_idt_entry(11, (u64int)_isr_handler_head_14, 0x08, 0x0, 0x8E);
	create_idt_entry(12, (u64int)_isr_handler_head_14, 0x08, 0x0, 0x8E);
	create_idt_entry(17, (u64int)_isr_handler_head_14, 0x08, 0x0, 0x8E);
	create_idt_entry(30, (u64int)_isr_handler_head_14, 0x08, 0x0, 0x8E);


	create_idt_entry(128, (u64int) syscall_handler, 0x08, 0x0, 0xEE);
	idtr_ptr.table_limit = sizeof(idt_entry_t)*256;
	idtr_ptr.linear_base_addr = (u64int)idt_entries;
       	_flush_idt((u64int)&idtr_ptr);	
}

void create_idt_entry(s32int num,
			     u64int target_offset,
			     u16int target_selector,
			     u8int ist,
			     u8int access_flags)
{
	idt_entries[num].target_offset_low = (target_offset & 0xFFFF);
	idt_entries[num].target_selector = target_selector;
	idt_entries[num].ist_reserved_bits = 0x0;
	idt_entries[num].access_bits = access_flags;
	idt_entries[num].target_offset_mid = (target_offset >> 16) & 0xFFFF;
	idt_entries[num].target_offset_high = (target_offset >> 32) & 0xFFFFFFFF;
	idt_entries[num].reserved = 0x0;
}

/*
 * Function to initialize the Programmable Interrupt Controller.
 */
void initialize_pic()
{
	/*
	 * Firstly, we need to setup the PIC correctly.
	 * This involves remapping the irq numbers, since
	 * there is an overlap with the IRQs and the processor
	 * defined exceptions and faults numbers.
	 * There is a Master and a Slave PIC, so we need to
	 * setup both. 
	 * The setup involves sending ICW0-ICW4 to these 
	 * PIC's software ports. The mappings are given below -
	 * 0x20 - Master PIC Command and Status Register
	 * 0x21 - Master  PIC Interrupt Mask Register and Data Register
	 * 0xA0 - Slave PIC Command and Status Register
	 * 0xA1 - Slave PIC Interrupt Mask Register and Data Register
	 *
	 * Whether we are reading or writing decides which of the
	 * two possible registers is used, for each mapping.
	 */
	
	__asm__(
		/* ICW1 to inform intent to initialize */
		"mov $0x11, %al\n\t"
		"out %al, $0x20\n\t"
		"out %al, $0xA0\n\t"
		/*
		  ICW2 to remap the base address of the IVT --
		  Master maps to 32(0x20) onwards and slave 40(0x28)
		  onwards.
		 */
		"mov $0x20, %al\n\t"
		"out %al, $0x21\n\t"
		"mov $0x28, %al\n\t"
		"out %al, $0xA1\n\t"
		/* 
		 * ICW3 to indicate which IRQ line is used to 
		 * communicate between the PICs.
		 * x86 says that Master should use line 2.
		 * To denote this we put 0x4 in Master and
		 * 0x2 in Slave.
		 */
		"mov $0x4, %al\n\t"
		"out %al, $0x21\n\t"
		"mov $0x2, %al\n\t"
		"out %al, $0xA1\n\t"
		/*
		 * ICW4 to control the operation.
		 * 1 indicates 80x86 mode as opposed to 
		 * 8086 mode.
		 */
		"mov $0x1, %al\n\t"
		"out %al, $0x21\n\t"
		"out %al, $0xA1\n\t"
		/*
		 * Clear the Interrupt Mask Registers
		 * so that interrupts can come through.
		 */
		"mov $0xfc, %al\n\t"
		/* 		"mov $0xfc, %al\n\t"*/
		"out %al, $0x21\n\t"
		"mov $0xff, %al\n\t"
		"out %al, $0xA1\n\t"
		/* 
		 * Enable INTR later in main.c
		 */
		/*"sti\n\t"*/
		);
}
