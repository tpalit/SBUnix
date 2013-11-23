/*
 * This file will contain the structures for the 
 * four levels of paging hierarchy - PML4, PDP, PD, PT
 * and any other helping structures.
 */

#ifndef VMMGR_H
#define VMMGR_H

#include<common.h>

/* The control register CR3 structure. */
typedef u64int cr3_reg;

/* The Page Map Level 4 structure. */
typedef u64int pml4_e;

/* The Page Directory Pointer structure. */
typedef u64int pdp_e;

/* The Page Directory structure. */
typedef u64int pd_e;

/* The Page Table structure. */
typedef u64int pt_e;


pml4_e* create_pml4_e(pml4_e*, u64int, u8int, u8int, u8int);
pdp_e* create_pdp_e(pdp_e*, u64int, u8int, u8int, u8int);
pd_e* create_pd_e(pd_e*, u64int, u8int, u8int, u8int);
pt_e* create_pt_e(pt_e*, u64int, u8int, u8int, u8int);
cr3_reg* create_cr3_reg(cr3_reg*, u64int, int, int);

u64int* set_base_addr(u64int*, u64int);
int is_present(u64int);
void set_present(u64int*);
u64int find_base_addr(u64int);

void init_pdp_tbl(u64int*);
void init_pd_tbl(u64int*);
void init_pt_tbl(u64int*);

/* Macros for the recursive slot */
#define PML4_REC_SLOT 510UL

#define PML4E_OFFSET(addr) ((((u64int)(addr))>>39) & 511)
#define PDPT_OFFSET(addr) ((((u64int)(addr))>>30) & 511)
#define PD_OFFSET(addr) ((((u64int)(addr))>>21) & 511)
#define PT_OFFSET(addr) ((((u64int)(addr))>>12) & 511)

#define BASE_ADDR_PT (0xFFFF000000000000 +(PML4_REC_SLOT<<39))
#define BASE_ADDR_PD (BASE_ADDR_PT + (PML4_REC_SLOT<<30))
#define BASE_ADDR_PDPT (BASE_ADDR_PD + (PML4_REC_SLOT<<21))
#define BASE_ADDR_PML4 (BASE_ADDR_PDPT + (PML4_REC_SLOT<<12))

/* Structures for tables. 
   u64int ptr = PT_ENTRY(addr);
   pt_e pte_entry = ptr[PT_OFFSET(addr)]; 
   u64int ptr1 = PD_ENTRY(aaddr);
   pd_e pde_entry = ptr2[PD_OFFSET(addr)];   
*/
#define PML4_ENTRY(addr) (/*(u64int*)*/BASE_ADDR_PML4)
#define PDP_ENTRY(addr) (/*(u64int*)*/(BASE_ADDR_PDPT + (((addr)>>27) & 0x00001FF000)))
#define PD_ENTRY(addr) (/*(u64int*)*/(BASE_ADDR_PD + (((addr)>>18) & 0x003FFFF000)))
#define PT_ENTRY(addr) (/*(u64int*)*/(BASE_ADDR_PT + (((addr)>>9) & 0x7FFFFFF000)))

/* The physical and virtual address of a page */
struct phys_vir_addr
{
	u64int phys_addr;
	u64int vir_addr;
};

typedef struct phys_vir_addr phys_vir_addr;

#define CODE_VMA 0
#define DATA_VMA 1
#define HEAP_VMA 2
#define STACK_VMA 3
#define OTHER_VMA 127

struct vm_struct{
	u8int vm_type;  /* The type of the vma - CODE, DATA, HEAP */
	u64int vm_start;      /* VMA start, inclusive */
	u64int vm_end;        /* VMA end , exclusive */
	struct vm_struct *vm_next;      /* list of VMA's */
};

typedef struct vm_struct vm_struct;

struct mm_struct{
	vm_struct* vm_head;
};

typedef struct mm_struct mm_struct;

void map_phys_vir_range(u64int, u64int, u64int);
void map_phys_vir_pg(u64int, u64int);
void init_pg_dir_pages(pml4_e*);

void* kmmap(void*, u64int, u32int, u32int, u32int, u32int);
u64int get_free_page(void);

/* For the rare cases, when the physical address of a page is needed. */
phys_vir_addr* get_free_phys_page(void);
#endif
