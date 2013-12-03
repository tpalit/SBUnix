/* 
 * This file will contain the structures for free, wired and zero 
 * page-frame lists.
 */

#ifndef PMMGR_H
#define PMMGR_H

#include<common.h>

#define PAGE_SIZE 4096

/* The constants for type of page */
#define KERN_PG 1
#define ANY_PG 2

/*
 * The structure for the non-zeroed page frames.
 */
struct free_pgfrm
{
	u64int pgfrm_saddr;
	struct free_pgfrm* next_pgfrm;
};

typedef struct free_pgfrm free_pgfrm;
typedef struct free_pgfrm* nz_free_list;
typedef struct free_pgfrm* z_free_list;
typedef struct free_pgfrm* wired_list;

typedef int TYPE_FRM;

void init_pg_frames(u64int, u64int, void**, void**);

nz_free_list alloc_phys_pg(TYPE_FRM);
void free_phys_pg(free_pgfrm*, TYPE_FRM);

u64int get_pg_aligned_addr(u64int);

#endif
