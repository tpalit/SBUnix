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

	int i = 0;
	for(i=0; i<512; i++){
		if (i==PML4_REC_SLOT) {
			/* The recursive mapping */
			create_pml4_e(&pml_entries_ptr[i], (u64int)pml4e_page->phys_addr, 0x0, 0x03, 0x00);
		} else {
			pml_entries_ptr[i] = pml_entries[i];			
		}
	}

	cr3_reg process_cr3;
	create_cr3_reg(&process_cr3, (u64int)pml_entries_phys_addr, 0x00, 0x00);
	child_task->cr3_register = process_cr3;
	/*
	__asm__ __volatile__(
			     "movq %%cr3, %0\n\t"
			     :"=r"(old_cr3));
	__asm__ __volatile__(
			     "movq %0, %%cr3\n\t"
			     ::"r"(child_task->cr3_register));
	*/
	kprintf("HERE\n");
	/* Map the parent's pages */
	while(parent_vma_ptr != NULL){
		kprintf("vm_start = %p, vm_end = %p", parent_vma_ptr->vm_start, parent_vma_ptr->vm_end);
		for(i4 = parent_vma_ptr->vm_start; i4 < parent_vma_ptr->vm_end; i4 += PML4_RANGE){
			/* For each pml4 entry, allocate a pdp table and set it's base address. */
			phys_vir_addr* pdp_page = get_free_phys_page();
			pdp_entries_ptr = (pdp_e*)pdp_page->vir_addr;
			pdp_entries_phys_addr = pdp_page->phys_addr;
			//			kprintf("The pdp entries phys address = %p", pdp_page->phys_addr);
			init_pdp_tbl(pdp_entries_ptr);
			kprintf("i4 = %p PML4E offset = %x", i4, PML4E_OFFSET(i4));
			set_base_addr(&pml_entries[PML4E_OFFSET(i4)], pdp_entries_phys_addr);
			set_present(&pml_entries[PML4E_OFFSET(i4)]);
			//			kprintf("PML wrote = %p\n", pml_entries_ptr[PML4E_OFFSET(i2)]);
			for(i3 = i4; i3 < i4+PML4_RANGE && i3 < parent_vma_ptr->vm_end; i3 += PDP_RANGE){
				/* For each pdp entry, allocate a pd table and set it's base address. */
				phys_vir_addr* pd_page = get_free_phys_page();
				pd_entries_ptr = (pd_e*)pd_page->vir_addr;
				pd_entries_phys_addr = pd_page->phys_addr;
				//	kprintf("The pd entries phys address = %p", pd_page->phys_addr);
				init_pd_tbl(pd_entries_ptr);
				kprintf("i3 = %p, PDPT offset = %x", i3, PDPT_OFFSET(i3));
				set_base_addr(&pdp_entries_ptr[PDPT_OFFSET(i3)], pd_entries_phys_addr);
				set_present(&pdp_entries_ptr[PDPT_OFFSET(i3)]);
				kprintf("PDP wrote = %p", pdp_entries_ptr[PDPT_OFFSET(i2)]);
				for(i2 = i3; i2 < i3+PDP_RANGE && i2 < parent_vma_ptr->vm_end; i2 += PD_RANGE){
					/* For each pd entry, allocate a pt table and set it's base address. */					
					phys_vir_addr* pt_page = get_free_phys_page();
					pt_entries_ptr = (pt_e*)pt_page->vir_addr;
					pt_entries_phys_addr = pt_page->phys_addr;
					//	kprintf("The pt entries phys address = %p", pt_page->phys_addr);
					init_pt_tbl(pt_entries_ptr);
					kprintf("i2 = %p, PD offset = %x", i2, PD_OFFSET(i2));
					set_base_addr(&pd_entries_ptr[PD_OFFSET(i2)], pt_entries_phys_addr);
					set_present(&pd_entries_ptr[PD_OFFSET(i2)]);
					kprintf("PD wrote = %p", pd_entries_ptr[PD_OFFSET(i2)]);
					for(i1 = i2; i1 < i2+PD_RANGE && i1 < parent_vma_ptr->vm_end; i1+= PT_RANGE){
						/* For each pt entry, copy it!*/
						kprintf("i1 = %p, PT offset = %x", i1, PT_OFFSET(i1));
						kprintf("pt_entries_ptr = %p", pt_entries_ptr);
						pt_e* page_table_entry = (pt_e*)PT_ENTRY(i1);
						pt_entries_ptr[PT_OFFSET(i1)] = page_table_entry[PT_OFFSET(i1)];
						kprintf("PT Wrote = %p", pt_entries_ptr[PT_OFFSET(i1)]);
					}
				}
			}
		}
		parent_vma_ptr = parent_vma_ptr->vm_next;
		kprintf("\n");
	}
	/*
	__asm__ __volatile__(
			     "movq %0, %%cr3\n\t"
			     ::"r"(old_cr3));	
	*/
	test(child_task);
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
	child_task->kernel_stack[126] = stack_vma->vm_end;
	child_task->kernel_stack[125] = DEFAULT_FLAGS;
	child_task->kernel_stack[124] = 0x1b;
	kprintf("return to %p", syscall_ret_address);
	child_task->kernel_stack[123] = syscall_ret_address;
	
	/* Pretend that the GP registers and one function call is also on the stack 
	 * %rax gets 0x0 by default. */
	int indx = 122;
	for (; indx>108; indx--) {
		child_task->kernel_stack[indx] = 0x0; 
	}
	child_task->rsp_register = (u64int)&child_task->kernel_stack[108];
}
