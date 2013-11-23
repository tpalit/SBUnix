/* 
 * This file will contain all the functions for creating and manipulating
 * the four level paging structure.
 */

#include<common.h>
#include<sys/vm_mgr.h>
#include<sys/pm_mgr.h>
#include<stdio.h>
/* Defined in main.c */
extern pml4_e pml_entries[];
extern pdp_e pdp_entries[];
extern pd_e pd_entries[];
extern pt_e pt_entries[];

/* Defined in kprintf.c */
extern u64int VIDEO_MEM_START;

/* The top of the virtual memory */
u64int VIR_MEM_TOP = 0;

/* 
 * Create a PML4E entry.
 */
pml4_e* create_pml4_e(pml4_e* pml4_ptr, u64int pdp_e_base_addr, u8int avl, u8int flags, 
		      u8int nx_bit)
{
	if(pdp_e_base_addr > KERN_VIR_START) {
		pdp_e_base_addr -= KERN_VIR_START;
	}
	*pml4_ptr = 0x0;
	*pml4_ptr |= ((u64int)nx_bit << 63);
	*pml4_ptr |= (pdp_e_base_addr & 0xfffffffffffff000);
	*pml4_ptr |= (avl << 9);
	*pml4_ptr |= (flags & 0x7F);
	return pml4_ptr;
}

/*
 * Create a PDP entry.
 */
pdp_e* create_pdp_e(pdp_e* pdp_ptr, u64int pd_e_base_addr, u8int avl, u8int flags, 
		    u8int nx_bit)
{
	if(pd_e_base_addr > KERN_VIR_START) {
		pd_e_base_addr -= KERN_VIR_START;
	}
	*pdp_ptr = 0x0;
	*pdp_ptr |= ((u64int)nx_bit << 63);
	*pdp_ptr |= (pd_e_base_addr & 0xfffffffffffff000);
	*pdp_ptr |= (avl << 9);
	*pdp_ptr |= (flags & 0x7F);
	return pdp_ptr;
}

/*
 * create a PD entry.
 */
pd_e* create_pd_e(pd_e* pd_ptr, u64int pt_e_base_addr, u8int avl, u8int flags,
		  u8int nx_bit)
{
	if(pt_e_base_addr > KERN_VIR_START) {
		pt_e_base_addr -= KERN_VIR_START;
	}
	*pd_ptr = 0x0;
	*pd_ptr |= ((u64int)nx_bit << 63);
	*pd_ptr |= (pt_e_base_addr & 0xfffffffffffff000);
	*pd_ptr |= (avl << 9);
	*pd_ptr |= (flags & 0x7F);
	return pd_ptr;
}

/*
 * Create a PT entry.
 */
pt_e* create_pt_e(pt_e* pt_ptr, u64int phys_base_addr, u8int avl, u8int flags,
		  u8int nx_bit)
{
	if(phys_base_addr > KERN_VIR_START) {
		phys_base_addr -= KERN_VIR_START;
	}
	*pt_ptr = 0x0;
	*pt_ptr |= ((u64int)nx_bit << 63);
	*pt_ptr |= (phys_base_addr & 0xfffffffffffff000);
	*pt_ptr |= (avl << 9);
	*pt_ptr |= (flags & 0x7F);
	return pt_ptr;
}

void init_pdp_tbl(u64int* pdp_entries)
{
	int i = 0;
	for (i=0; i<512; i++) {
		create_pdp_e(&pdp_entries[i], 0x0, 0x0, 0x06, 0x00);		
	}
}

void init_pd_tbl(u64int* pd_entries)
{
	int i = 0;
	for (i=0; i<512; i++) {
		create_pd_e(&pd_entries[i], 0x0, 0x0, 0x06, 0x00);		
	}
}

void init_pt_tbl(u64int* pt_entries)
{
	int i = 0;
	for (i=0; i<512; i++) {
		create_pt_e(&pt_entries[i], 0x0, 0x0, 0x06, 0x00);		
	}

}

/* 
 * Populate the contents of the cr3 register.
 */
cr3_reg* create_cr3_reg(cr3_reg* cr3_reg, u64int pml4e_tbl_base, int pcd, int pwt)
{
	*cr3_reg = 0x0;
	*cr3_reg |= ((pwt << 3) & 0x08);
	*cr3_reg |= (pcd << 4);
	*cr3_reg |= (pml4e_tbl_base & 0xfffffffffffff000);
	return cr3_reg;
}

int is_present(u64int entry)
{
	return entry&0x01;
}

void set_present(u64int* entry)
{
	*entry |= 0x01;
}
/*
 * This file will extract the "base address" field
 * of any of the PML4E, PDPE, PDE or PTE entries.
 * "base address" should be 40 bits.
 */
u64int find_base_addr(u64int entry)
{
	return ((entry >> 12) & 0xffffffffff);
}

/*
 * This function will set the "base address" field of 
 * any of the PML4E, PDPE, PDE or PTE entries.
 * "base address" should be 40 bits.
 */
u64int* set_base_addr(u64int* entry, u64int base_addr) 
{
	if(base_addr > KERN_VIR_START) {
		base_addr -= KERN_VIR_START;
	}
	*entry |= (base_addr & 0xffffffffff000);
	return entry;
}

/**
 * Map the kernel's static (as opposed to "dynamic") memory. 
 */
void static_map_pg(u64int vir_pg, u64int phys_pg)
{
	pml4_e* pml4e_entry;
	pdp_e* pdp_entry;
	pd_e* pd_entry;
	
	u16int pml4e_offset = 0x0;
	u16int pdp_offset = 0x0;
	u16int pd_offset = 0x0;
	u16int pt_offset = 0x0;

	pml4e_offset = ((vir_pg>>39)&0x1ff);
	pdp_offset = ((vir_pg>>30)&0x1ff);
	pd_offset = ((vir_pg>>21)&0x1ff);
	pt_offset = ((vir_pg>>12)&0x1ff);

	pml4e_entry = &pml_entries[pml4e_offset];
		
	/* The PDPE level */
	if(is_present((u64int)pml4e_entry)){
		pdp_entry = &pdp_entries[pdp_offset];
	} else {
		/* Create a new pdpe entry*/
		pdp_entry = create_pdp_e(&pdp_entries[pdp_offset], (u64int)&pd_entries, 0x0, 0x06, 0x00); // pd is not yet created
		set_base_addr(pml4e_entry, (u64int)pdp_entries);
		set_present(pml4e_entry);
	}

	/* The PD level */
	if(is_present((u64int)pdp_entry)){
		pd_entry = &pd_entries[pd_offset];
	} else {
		/* Create new pde entry */
		pd_entry = create_pd_e(&pd_entries[pd_offset], (u64int)&pt_entries, 0x0, 0x06, 0x00);
		set_base_addr(pdp_entry, (u64int)pd_entries);
		set_present(pdp_entry);
	}

	/* The PT level */
	
	/* Create new pte entry */
	if(!is_present((u64int)pd_entry)){
		create_pt_e(&pt_entries[pt_offset], phys_pg, 0x0, 0x07, 0x00);
		set_base_addr(pd_entry, (u64int)pt_entries);
		set_present(pd_entry);
	}

}

/**
 * This function will map a range of physical pages to virtual pages.
 * It will create all the four levels of page table hierarchy.
 */
void map_phys_vir_range(u64int phys_pg_start, u64int phys_pg_end, u64int vir_pg_start)
{
	
	u64int phys_pg = (u64int)phys_pg_start;
	u64int vir_pg = (u64int)vir_pg_start;
	for(; phys_pg <= (u64int)phys_pg_end; phys_pg = phys_pg+PAGE_SIZE, vir_pg=vir_pg+PAGE_SIZE) {
		static_map_pg(vir_pg, phys_pg);
	}
	static_map_pg(vir_pg, 0xB8000);
       	VIDEO_MEM_START = vir_pg;
	VIR_MEM_TOP = vir_pg+PAGE_SIZE; /* Update the top of the virtual memory */
}

void init_pg_dir_pages(pml4_e *pml4_entries)
{
	int i = 0;

	/* Create blank PML4E */
	for(i=0; i<512; i++){
		/*
		  P - Unset
		  R/W - Set (both)
		  U/S - Set (both)
		  PWT - Set (Writethrough)
		  PCD - Set
		  A - Unset
		*/
		if (i==510) {
			/* The recursive mapping */
			create_pml4_e(&pml4_entries[i], (u64int)pml4_entries, 0x0, 0x07, 0x00);
		} else {
			create_pml4_e(&pml4_entries[i], 0x0, 0x0, 0x06, 0x00);
		}
	}
	/* Create blank PDPE table */
	for(i=0; i<512; i++){
		create_pdp_e(&pdp_entries[i], 0x0, 0x0, 0x06, 0x00);
	}
	/* Create blank PD table */
	for(i=0; i<512; i++){
		create_pd_e(&pd_entries[i], 0x0, 0x0, 0x06, 0x00);
	}
	/* Create blank PT table */
	for(i=0; i<512; i++){
		create_pt_e(&pt_entries[i], 0x0, 0x0, 0x06, 0x00);
	}
}

/**
 * Map dynamically allocated memory. get_free_page(), etc.
 */
void map_phys_vir_pg(u64int phys_addr, u64int vir_addr)
{
	u64int* pt_ptr = 0;
	pt_e* pt_entry_ptr = 0;

	u64int* pd_ptr = 0;
	pd_e* pd_entry_ptr = 0;

	u64int* pdp_ptr = 0;
	pdp_e* pdp_entry_ptr = 0;

	u64int* pml4_ptr = (u64int*)PML4_ENTRY(vir_addr);

	pml4_e* pml4_entry_ptr = pml4_ptr+PML4E_OFFSET((u64int)vir_addr);

	nz_free_list free_page;
	
	if(is_present((u64int)*pml4_entry_ptr)){
		pdp_ptr = (u64int*)PDP_ENTRY(vir_addr);
		pdp_entry_ptr = pdp_ptr+PDPT_OFFSET((u64int)vir_addr);
	} else {
		/* Allocate a new PDP table */
		free_page = alloc_phys_pg(KERN_PG);
		set_present(pml4_entry_ptr);
		set_base_addr(pml4_entry_ptr, free_page->pgfrm_saddr);
		pdp_ptr = (u64int*)PDP_ENTRY(vir_addr);
		/* Initialize the values */
		init_pdp_tbl(pdp_ptr);
		pdp_entry_ptr = pdp_ptr+PDPT_OFFSET((u64int)vir_addr);
		create_pdp_e(pdp_entry_ptr, 0x0, 0x0, 0x06, 0x0); 
	}
	if(is_present((u64int)*pdp_entry_ptr)){
		pd_ptr = (u64int*)PD_ENTRY(vir_addr);
		pd_entry_ptr = pd_ptr+PD_OFFSET((u64int)vir_addr);
	} else {
		/* Allocate a new PD table */
		free_page = alloc_phys_pg(KERN_PG);
		set_present(pdp_entry_ptr);
		set_base_addr(pdp_entry_ptr, free_page->pgfrm_saddr);
		pd_ptr = (u64int*)PD_ENTRY(vir_addr);
		/* Initialize the values */
		init_pd_tbl(pd_ptr);
		pd_entry_ptr = pd_ptr+PD_OFFSET((u64int)vir_addr);
		create_pd_e(pd_entry_ptr, 0x0, 0x0, 0x06, 0x0);
	}

	if(is_present((u64int)*pd_entry_ptr)){
		pt_ptr = (u64int*)PT_ENTRY(vir_addr);
		pt_entry_ptr = pt_ptr+PT_OFFSET((u64int)vir_addr);
	} else {
		/* Allocate a new PT table */
		free_page = alloc_phys_pg(KERN_PG);
		set_present(pd_entry_ptr);
		set_base_addr(pd_entry_ptr, free_page->pgfrm_saddr);
		pt_ptr = (u64int*)PT_ENTRY(vir_addr);
		/* Initialize the values */
		init_pt_tbl(pt_ptr);
		pt_entry_ptr = pt_ptr+PT_OFFSET((u64int)vir_addr);
		create_pt_e(pt_entry_ptr, 0x0, 0x0, 0x06, 0x0);
	}
	if(is_present((u64int)*pt_entry_ptr)){
		panic("Tried to map an already mapped page.");
	} else {
		create_pt_e(pt_entry_ptr, 0x0, 0x0, 0x06, 0x0);
		set_present(pt_entry_ptr);
		set_base_addr(pt_entry_ptr, phys_addr);
	}
}

/*
 * This function is used to get a new free page. 
 * This function will first call the physical memory manager to 
 * get a free page. Then it'll map the free page to the top of the virtual 
 * address space (@TODO - We aren't reclaiming virtual address space now.)
 * Then it'll return the virtual address of the page.
 *
 * kmalloc will build on this. 
 */ 
u64int get_free_page(void)
{
	nz_free_list free_page = alloc_phys_pg(KERN_PG);
	VIR_MEM_TOP += PAGE_SIZE;
       	map_phys_vir_pg(free_page->pgfrm_saddr, VIR_MEM_TOP);
	return VIR_MEM_TOP;
}

phys_vir_addr page_addr;
/*
 * This should never be used from outside the kernel. It uses a globally 
 * allocated structure, whose data might get corrupted by other threads. 
 */
phys_vir_addr* get_free_phys_page(void)
{
	nz_free_list free_page = alloc_phys_pg(KERN_PG);
	VIR_MEM_TOP += PAGE_SIZE;
       	map_phys_vir_pg(free_page->pgfrm_saddr, VIR_MEM_TOP);
	page_addr.phys_addr = free_page->pgfrm_saddr;
	page_addr.vir_addr = VIR_MEM_TOP;
	return &page_addr;
}

/**
 * The kernel mmap. 
 * @TODO - Handles consecutive anonymous mappings only now.
 */
void* kmmap(void* start_addr, u64int length, u32int prot, u32int flags, u32int fd, u32int offset)
{
	u64int addr = (u64int)start_addr;
	u64int bytes_allocated = 0;
	nz_free_list free_page = NULL;
	while(bytes_allocated < length) {
		free_page = alloc_phys_pg(KERN_PG);
		map_phys_vir_pg(free_page->pgfrm_saddr, (u64int)addr);
		bytes_allocated += PAGE_SIZE;
		addr += PAGE_SIZE;
	}
	return start_addr;
}

/*
 * This function will free a page.
 * It will call unmap_phys_vir_pg() to do so.
 */
void free_page(u64int vir_addr)
{
	
}
