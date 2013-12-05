/**
 * Copyright (c) 2013 by Tapti Palit, Kaustubh Gharpure. All rights reserved.
 */
/*
 * The much required kmalloc function!
 */

#include<sys/kmalloc.h>
#include<stdio.h>
#include<sys/vm_mgr.h>
#include<sys/pm_mgr.h>
#include<common.h>

u64int km_top_addr = NULL; 
u64int km_curr_pg_base = NULL;

/*
 * Kmalloc to allocate less than one page of memory. 
 */
void* kmalloc(u32int k_size)
{
	if (k_size > PAGE_SIZE) {
		panic("Can't allocate more than one page by kmalloc, right now!");
	}
	void* ret_ptr = (void*)km_top_addr;
	if (km_top_addr == NULL || km_curr_pg_base == NULL 
	    || km_top_addr+k_size > km_curr_pg_base+PAGE_SIZE){
		km_curr_pg_base = get_free_page();
		km_top_addr = km_curr_pg_base;
		ret_ptr = (void*)km_top_addr;
	} 
	km_top_addr += k_size;
	return ret_ptr;
}

