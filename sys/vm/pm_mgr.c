/*
 * This is the physical memory manager.
 * This file will deal with the physical memory management. 
 * Mainly with the breaking up of the physical memory into chunks 
 * and maintaining them in free lists. 
 * It will also contain the code for the alloc_phys_pg and free_phys_pg.
 *
 * There will be three lists maintained - free-zeroed list, free-unzeroed list, 
 * and the wired list. 
 */

#include<common.h>
#include<sys/pm_mgr.h>

/* The zeroed, non-zeroed, wired free lists. */
z_free_list z_free_head = NULL;
nz_free_list nz_free_head = NULL;
wired_list wired_head = NULL;

/* Defined in kmalloc.c */
extern u64int km_top_add; 
extern nz_free_list km_curr_pg;


void zero_out_page(nz_free_list page_frame)
{
	u8int* ptr = (u8int*)page_frame->pgfrm_saddr;
	int i=0;
	for(; i<PAGE_SIZE; i++){
		*(ptr+i) = 0xAB;
	}
}
/* 
 * This function will divide the available free memory 
 * into "chunks" or "page frames" of 4kb each. Then it will
 * create the free list for it. 
 * This free list data structures would be on top of the kernel
 * that is - just above the physfree pointer. 
 * Once we are done with allocating it and know how much space is
 * needed for the data structure, we'll increment physfree by that
 * much offset.
 */
void init_pg_frames(u64int free_start, u64int free_end, 
		    void** phys_base_pptr, void** phys_free_pptr) 
{
	u64int mem_traverser_ptr = NULL;
	u64int start_phys_free = NULL;
	nz_free_list nz_free_item = NULL;
	nz_free_list nz_prev_free_item = NULL;

	/* We'll keep aside memory for free_mem/PAGESIZE entries and
	   update the phys_free pointer. 
	   @TODO - Only non-zero free lists are being handled now.
	*/
	nz_free_item = (nz_free_list)(*phys_free_pptr);

       	u64int mem_size = free_end - (u64int)*phys_free_pptr;
	start_phys_free = (u64int)(*phys_free_pptr)+(mem_size/PAGE_SIZE)*sizeof(free_pgfrm);
	nz_free_head = (nz_free_list)(KERN_VIR_START+(u64int)nz_free_item); 
	/* Start mapping from the new free pointer */
	for(mem_traverser_ptr = ((start_phys_free+PAGE_SIZE)&0xfffffffffffff000); 
	    mem_traverser_ptr < free_end;
	    mem_traverser_ptr += PAGE_SIZE){
		nz_free_item->pgfrm_saddr = mem_traverser_ptr;
		nz_free_item->next_pgfrm = NULL;
		if (nz_prev_free_item != NULL) {
			nz_prev_free_item->next_pgfrm = (nz_free_list)(KERN_VIR_START+(u64int)nz_free_item);
		}
		nz_prev_free_item = nz_free_item;
		//		zero_out_page(nz_free_item);
		nz_free_item+=sizeof(free_pgfrm);
	}

	*phys_free_pptr = (void*)start_phys_free;
}

/*
 * Allocate a new physical page. The TYPE_FRM decides whether
 * it is to be marked as a kernel page or not. 
 */
nz_free_list alloc_phys_pg(TYPE_FRM frame_type)
{
	nz_free_list nz_free_pg;
	if (nz_free_head->next_pgfrm == NULL) {
		panic("Out of physical memory!");
		return NULL;
	}
	nz_free_pg = nz_free_head->next_pgfrm;
	nz_free_head = nz_free_head->next_pgfrm;
	return nz_free_pg;
}

/*
 * Free a physical page.
 */
void free_phys_pg(free_pgfrm* pg_frm, TYPE_FRM frame_type) 
{
	pg_frm->next_pgfrm = nz_free_head;
	nz_free_head = pg_frm;
}

u64int get_pg_aligned_addr(u64int addr){
	if (addr % PAGE_SIZE != 0) {
	        addr += PAGE_SIZE;
		addr &= 0xfffffffffffff000;
	}
	return addr;
}
