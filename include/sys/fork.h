/* 
 * The header file for fork()
 */

#ifndef FORK_H
#define FORK_H

#include<sys/kmalloc.h>
#include<sys/vm_mgr.h>
#include<sys/proc_mgr.h>
#include<common.h>

/* The range of a PT, PD, PDP and PML4 */
#define PML4_RANGE 512*512*512*4096UL
#define PDP_RANGE 512*512*4096UL
#define PD_RANGE 512*4096UL
#define PT_RANGE 4096UL

u64int do_fork(void);

task_struct* create_blank_process(void);
void copy_paging_structures(task_struct*, task_struct*);
void copy_vmas(task_struct*, task_struct*);
void setup_stack(task_struct*);

/* Caching the phys<->vir mapping */
struct mem_mapping_struct {
	u64int vir_addr;
	u64int phys_addr;
	struct mem_mapping_struct* next;
};
typedef struct mem_mapping_struct mem_mapping_struct;

void cache_mapping(phys_vir_addr*);
pdp_e* get_pdp_base_from_cache(pml4_e*);
pd_e* get_pd_base_from_cache(pdp_e*);
pt_e* get_pt_base_from_cache(pd_e*);

u64int find_vir_addr_from_cache(u64int phys_addr);
#endif
