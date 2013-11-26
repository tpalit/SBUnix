#include<sys/fork.h>
#include<sys/kmalloc.h>
#include<sys/vm_mgr.h>
#include<sys/proc_mgr.h>
#include<stdio.h>

extern task_struct* CURRENT_TASK;
extern u32int PROC_ID_TOP;
extern pml4_e pml_entries[512];
extern u64int syscall_ret_address;

/**
 * While forking, we need to be able to play with the paging structures of both the
 * parent and child process. And since we are using recursive mapping, I can't do 
 * that (unless we load and unload the %cr3 register each time). So, I'm maintaining
 * a cache of the vir<->phys mapping that will expire after all of parent's paging
 * structures is copied to the child. This is that cache.
 */
mem_mapping_struct* cache_head = NULL;

void cache_mapping(phys_vir_addr* phys_vir_pg) 
{
	mem_mapping_struct* cache_ptr = (mem_mapping_struct*)kmalloc(sizeof(mem_mapping_struct));
	mem_mapping_struct* cache_trav_ptr = cache_head;
	cache_ptr->vir_addr = phys_vir_pg->vir_addr;
	cache_ptr->phys_addr = phys_vir_pg->phys_addr;
	cache_ptr->next = NULL;
	if(cache_head == NULL){
		cache_head = cache_ptr;
	} else if(cache_head->next == NULL){
		cache_head->next = cache_ptr;
	} else {
		while(cache_trav_ptr->next != NULL){
			cache_trav_ptr = cache_trav_ptr->next;
		}
		cache_trav_ptr->next = cache_ptr;
	}
}

pdp_e* get_pdp_base_from_cache(pml4_e* pml4_entry_ptr)
{
	u64int phys_base_addr = find_base_addr((u64int)*pml4_entry_ptr);
	u64int vir_base_addr = find_vir_addr_from_cache(phys_base_addr);
	return (pdp_e*)vir_base_addr;
}

pd_e* get_pd_base_from_cache(pdp_e* pdp_entry_ptr)
{
	u64int phys_base_addr = find_base_addr((u64int)*pdp_entry_ptr);
	u64int vir_base_addr = find_vir_addr_from_cache(phys_base_addr);
	return (pd_e*)vir_base_addr;
}

pt_e* get_pt_base_from_cache(pd_e* pd_entry_ptr) 
{
	u64int phys_base_addr = find_base_addr((u64int)*pd_entry_ptr);
	u64int vir_base_addr = find_vir_addr_from_cache(phys_base_addr);
	return (pt_e*)vir_base_addr;
}

u64int find_vir_addr_from_cache(u64int phys_addr)
{
	mem_mapping_struct* cache_trav_ptr = cache_head;
	while(cache_trav_ptr != NULL){
		if (cache_trav_ptr->phys_addr == phys_addr){
			return cache_trav_ptr->vir_addr;
		}
		cache_trav_ptr = cache_trav_ptr->next;
	}
	panic("Trouble setting up pages for child!");
	return NULL; /* keep compiler happy */
}


/**
 * The heart of the forking functionality. 
 * Steps --
 * 1. Create a (new) child task_struct.
 *     Assign it a different pid.
 * 2. Copy the paging structures, but keep them pointing to same physical pages.
 *    Point the cr3_register for the child task_struct to this paging structure.
 *    Mark these pages as read only. 
 * 3. Copy the vmas as is from the parent.
 * 4. The stack is tricky. Need to set it so that it returns to the fork() instruction
 *    in the user program. The stack of the parent would have the pushes for all these
 *    functions in it. 
 *    Also need to push 0 as the value into rax.
 * 5. Add to the READY_LIST
 */
u64int do_fork(void)
{
	task_struct* child_task_ptr = create_blank_process();
	copy_paging_structures(CURRENT_TASK, child_task_ptr);
	copy_vmas(CURRENT_TASK, child_task_ptr);
	setup_stack(child_task_ptr);
	add_to_ready_list(child_task_ptr);
	return child_task_ptr->pid;
}


/**
 * Creates a blank process with a new process id.
 */
task_struct* create_blank_process(void)
{
	task_struct* child_task = (task_struct*)kmalloc(sizeof(task_struct));
	child_task->kernel_stack[127] = 0x23;
	child_task->kernel_stack[126] = BAD_ADDRESS; /* Bad address - should be overwritten later */
	child_task->kernel_stack[125] = DEFAULT_FLAGS;
	child_task->kernel_stack[124] = 0x1b;
	child_task->kernel_stack[123] = BAD_ADDRESS;

	/* Pretend that the GP registers and one function call is also on the stack */
	int indx = 122;
	for (; indx>108; indx--) {
		child_task->kernel_stack[indx] = 0x0;
	}
	child_task->time_slices = DEFAULT_TIME_SLICE;
	child_task->rsp_register = (u64int)&child_task->kernel_stack[108];
	child_task->rip_register = BAD_ADDRESS;
	child_task->rflags = DEFAULT_FLAGS;
	child_task->next = NULL;
	child_task->last_run = NULL;
	child_task->pid = PROC_ID_TOP++;
	child_task->vm_head = NULL;
	child_task->waiting_on = NULL;
	child_task->wait_time_slices = 0;
	child_task->cr3_register = BAD_ADDRESS; /* Bad address - to be overwritten later */
	return child_task;
}

void test(task_struct* ptr)
{
	__asm__ __volatile__(
			     "movq %0, %%cr3\n\t"
			     ::"r"(ptr->cr3_register));			

	u64int faulting_address = 0x400136;
	u64int* pml4_ptr = (u64int*)PML4_ENTRY(faulting_address);
	pml4_e* pml4_entry_ptr = pml4_ptr+PML4E_OFFSET((u64int)faulting_address);
	u64int* pdp_ptr = (u64int*)PDP_ENTRY(faulting_address);
	pdp_e* pdp_entry_ptr = pdp_ptr+PDPT_OFFSET((u64int)faulting_address);
	u64int* pd_ptr = (u64int*)PD_ENTRY(faulting_address);
	pd_e* pd_entry_ptr = pd_ptr+PD_OFFSET((u64int)faulting_address);
	u64int* pt_ptr = (u64int*)PT_ENTRY(faulting_address);
	pt_e* pt_entry_ptr = pt_ptr+PT_OFFSET((u64int)faulting_address);
	kprintf("pml4_entry_ptr = %p\n", *pml4_entry_ptr);
	kprintf("pdp_entry_ptr = %p\n", *pdp_entry_ptr);
	kprintf("pd_entry_ptr = %p\n", *pd_entry_ptr);
	kprintf("pt_entry_ptr = %p\n", *pt_entry_ptr);
	*(int*)faulting_address = 10;
	kprintf("Wrote %d", *(int*)faulting_address);
	while(1);
}


/**
 * Copy the paging structures. 
 * Iterate "nestedly" through the 4 levels, creating blank entries and copying 
 * the required values from the parent. 
 */
void copy_paging_structures(task_struct* parent_task, task_struct* child_task) 
{

	u64int i4 = 0x0;
	u64int i3 = 0x0;
	u64int i2 = 0x0;
	u64int i1 = 0x0;
	//	u64int old_cr3 = 0L;

	vm_struct* parent_vma_ptr = parent_task->vm_head;
	phys_vir_addr* pml4e_page = get_free_phys_page();

	/* Map the higher memory(kernel) by copying the PML4E*/
	pml4_e* pml_entries_ptr = (pml4_e*)pml4e_page->vir_addr;
	u64int pml_entries_phys_addr = pml4e_page->phys_addr;

	pdp_e* pdp_entries_ptr = NULL;
	u64int pdp_entries_phys_addr = NULL;
	pd_e* pd_entries_ptr = NULL;
	u64int pd_entries_phys_addr = NULL;
	pt_e* pt_entries_ptr = NULL;
	u64int pt_entries_phys_addr = NULL;

	init_pml_tbl(pml_entries_ptr);
	create_pml4_e(&pml_entries_ptr[PML4_REC_SLOT], (u64int)pml4e_page->phys_addr, 0x0, 0x03, 0x00);
	pml_entries_ptr[511] = pml_entries[511];

	pml4e_page = NULL;
	cr3_reg process_cr3;
	create_cr3_reg(&process_cr3, (u64int)pml_entries_phys_addr, 0x00, 0x00);
	child_task->cr3_register = process_cr3;

	/* Map the parent's pages */
	while(parent_vma_ptr != NULL){
		for(i4 = parent_vma_ptr->vm_start; i4 < parent_vma_ptr->vm_end; i4 += PML4_RANGE){
			/* For each pml4 entry, if not present allocate a pdp table and set it's base address. */
			if(!is_present((u64int)pml_entries_ptr[PML4E_OFFSET(i4)])){
				phys_vir_addr* pdp_page = get_free_phys_page();
				cache_mapping(pdp_page);
				pdp_entries_ptr = (pdp_e*)pdp_page->vir_addr;
				pdp_entries_phys_addr = pdp_page->phys_addr;
				pdp_page = NULL;
				init_pdp_tbl(pdp_entries_ptr);
				set_base_addr(&pml_entries_ptr[PML4E_OFFSET(i4)], pdp_entries_phys_addr);
				set_present(&pml_entries_ptr[PML4E_OFFSET(i4)]);
			} else {
				/*
				u64int* pdp_ptr_base = get_pdp_base_from_cache(&pml_entries_ptr[PML4E_OFFSET(i4)]);
				pdp_entries_ptr = pdp_ptr_base+PDPT_OFFSET((u64int)i4);
				*/
				pdp_entries_ptr = get_pdp_base_from_cache(&pml_entries_ptr[PML4E_OFFSET(i4)]);
			}
			for(i3 = i4; i3 < i4+PML4_RANGE && i3 < parent_vma_ptr->vm_end; i3 += PDP_RANGE){
				/* For each pdp entry, if not present, allocate a pd table and set it's base address. */
				if(!is_present((u64int)pdp_entries_ptr[PDPT_OFFSET(i3)])) {
					phys_vir_addr* pd_page = get_free_phys_page();
					cache_mapping(pd_page);
					pd_entries_ptr = (pd_e*)pd_page->vir_addr;
					pd_entries_phys_addr = pd_page->phys_addr;
					pd_page = NULL;
					init_pd_tbl(pd_entries_ptr);
					set_base_addr(&pdp_entries_ptr[PDPT_OFFSET(i3)], pd_entries_phys_addr);
					set_present(&pdp_entries_ptr[PDPT_OFFSET(i3)]);
				} else {
					/*
					u64int* pd_ptr_base = get_pd_base_from_cache(&pdp_entries_ptr[PDPT_OFFSET(i3)]);
					pd_entries_ptr = pd_ptr_base+PD_OFFSET((u64int)i3);
					*/
					pd_entries_ptr = get_pd_base_from_cache(&pdp_entries_ptr[PDPT_OFFSET(i3)]);
				}
				for(i2 = i3; i2 < i3+PDP_RANGE && i2 < parent_vma_ptr->vm_end; i2 += PD_RANGE){
					/* For each pd entry, if not present, allocate a pt table and set it's base address. */					
					if (!is_present((u64int)pd_entries_ptr[PD_OFFSET(i2)])) {
						phys_vir_addr* pt_page = get_free_phys_page();
						cache_mapping(pt_page);
						pt_entries_ptr = (pt_e*)pt_page->vir_addr;
						pt_entries_phys_addr = pt_page->phys_addr;
						pt_page = NULL;
						init_pt_tbl(pt_entries_ptr);
						set_base_addr(&pd_entries_ptr[PD_OFFSET(i2)], pt_entries_phys_addr);
						set_present(&pd_entries_ptr[PD_OFFSET(i2)]);
					} else {
						/*
						u64int* pt_ptr_base = get_pt_base_from_cache(&pd_entries_ptr[PD_OFFSET(i2)]);
						pt_entries_ptr = pt_ptr_base+PT_OFFSET((u64int)i2);
						*/
						pt_entries_ptr = get_pt_base_from_cache(&pd_entries_ptr[PD_OFFSET(i2)]);
					}
					for(i1 = i2; i1 < i2+PD_RANGE && i1 < parent_vma_ptr->vm_end; i1+= PT_RANGE){
						/* For each pt entry, copy it! If already present, panic*/
						if (!is_present((u64int)pt_entries_ptr[PT_OFFSET(i1)])) {
							pt_e* page_table_entry = (pt_e*)PT_ENTRY(i1);
							pt_entries_ptr[PT_OFFSET(i1)] = page_table_entry[PT_OFFSET(i1)];
						} else {
							panic("Tried to map an already mapped page!");
						}
					}
				}
			}
		}
		parent_vma_ptr = parent_vma_ptr->vm_next;
	}
	/* Clear the cached mappings. */
	cache_head = NULL;
	//	test(child_task);
}


/**
 * Copy the vmas from the parent to the child.
 */
void copy_vmas(task_struct* parent_task, task_struct* child_task)
{
	vm_struct* p_vm_ptr = parent_task->vm_head;
	vm_struct* c_vm_head = NULL;
	while(p_vm_ptr != NULL){
		vm_struct* c_vm_ptr = (vm_struct*)kmalloc(sizeof(vm_struct));
		c_vm_ptr->vm_type = p_vm_ptr->vm_type;
		c_vm_ptr->vm_start = p_vm_ptr->vm_start;
		c_vm_ptr->vm_end = p_vm_ptr->vm_end;
		c_vm_ptr->vm_next = NULL;
		/* Attach! */
		if (c_vm_head == NULL){
			c_vm_head = c_vm_ptr;
		} else {
			vm_struct* vm_trav_ptr = c_vm_head;
			while(vm_trav_ptr->vm_next != NULL){
				vm_trav_ptr = vm_trav_ptr->vm_next;
			}
			vm_trav_ptr->vm_next = c_vm_ptr;
		}
		p_vm_ptr = p_vm_ptr->vm_next;
	}
	child_task->vm_head = c_vm_head;
}

/**
 * Carefully set up the stack.
 */
void setup_stack(task_struct* child_task)
{
	/* Find the stack vma */
	vm_struct* stack_vma = NULL;
	vm_struct* vm_ptr = child_task->vm_head;
	while(vm_ptr != NULL){
		if(vm_ptr->vm_type == STACK_VMA){
			stack_vma = vm_ptr;
			break;
		}
		vm_ptr = vm_ptr->vm_next;
	}
	if (stack_vma == NULL) {
		panic("Parent process has no stack!");
	}
	/* Set up task parameters as per what IRETQ expects*/
	child_task->kernel_stack[127] = 0x23;
	/* 
	 * The compiler will pop off stuff as it is returning from a syscall.
	 * This includes the registers. @TODO - Shouldn't these be restored? Check later.
	 */
	child_task->kernel_stack[126] = 0xa00f28; 
	child_task->kernel_stack[125] = DEFAULT_FLAGS;
	child_task->kernel_stack[124] = 0x1b;
	child_task->kernel_stack[123] = syscall_ret_address;
	
	/* Pretend that the GP registers and one function call is also on the stack 
	 * %rax gets 0x0 by default. */
	int indx = 122;
	for (; indx>108; indx--) {
		child_task->kernel_stack[indx] = 0x0; 
	}
	child_task->rsp_register = (u64int)&child_task->kernel_stack[108];
}

