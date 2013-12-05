/*
 * This file contains the structures representing the segment 
 * descriptors for GDT, IDT, etc. 
 *
 * We'll use smart tricks to ensure that the memory 
 * alignment of the GDT descriptor structure is packed
 *
 * Copyright (c) 2013 by Tapti Palit, Kaustubh Gharpure. All rights reserved.
 */

#include<common.h>


struct gdt_entry_struct
{
	u16int seg_limit_low;
	u16int base_addr_low;
	u8int base_addr_mid;
	u8int access_misc_bits; /* Contains the TYPE, S, DPL and P bits. */
	u8int gran_misc_bits; /* Contains the Seg Limit(High), AVL, L, D/B, G bits */
	u8int base_addr_high;
}__attribute__((packed));

typedef struct gdt_entry_struct gdt_entry_t;

struct sys_segment_descriptor {
	u64int sd_lolimit:16;/* segment extent (lsb) */
	u64int sd_lobase:24; /* segment base address (lsb) */
	u64int sd_type:5;    /* segment type */
	u64int sd_dpl:2;     /* segment descriptor priority level */
	u64int sd_p:1;       /* segment descriptor present */
	u64int sd_hilimit:4; /* segment extent (msb) */
	u64int sd_xx1:3;     /* avl, long and def32 (not used) */
	u64int sd_gran:1;    /* limit granularity (byte/page) */
	u64int sd_hibase:40; /* segment base address (msb) */
	u64int sd_xx2:8;     /* reserved */
	u64int sd_zero:5;    /* must be zero */
	u64int sd_xx3:19;    /* reserved */
}__attribute__((packed));

typedef struct sys_segment_descriptor sys_segment_descriptor;

struct tss_struct {
	u32int reserved;
	u64int rsp0;
	u32int unused[11];
}__attribute__((packed));

typedef struct tss_struct tss_struct;

struct gdtr_pointer_struct
{
	u16int table_limit; /* The limit of the GDT table. */
	u64int linear_base_addr; /* The base address of the GDT table */
}__attribute__((packed));

typedef struct gdtr_pointer_struct gdtr_pointer;

/* The IDT is 128 bytes in IA32e mode. */
struct idt_entry_struct
{
	u16int target_offset_low;
	u16int target_selector;
	u8int ist_reserved_bits;
	u8int access_bits;
	u16int target_offset_mid;
	u32int target_offset_high;
	u32int reserved;
}__attribute__((packed));

typedef struct idt_entry_struct idt_entry_t;

struct idtr_pointer_struct
{
	u16int table_limit; /* The limit */
	u64int linear_base_addr; /* The base address of IDT table */
}__attribute__((packed));

typedef struct idtr_pointer_struct idtr_pointer;

void init_descriptor_tables();
