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

#endif
